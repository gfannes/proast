#ifndef HEADER_proast_presenter_Presenter_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_Presenter_hpp_ALREADY_INCLUDED

#include <proast/presenter/Commander.hpp>
#include <proast/presenter/ListBox.hpp>
#include <proast/model/Model.hpp>
#include <proast/view/View.hpp>
#include <gubg/mss.hpp>
#include <gubg/Range.hpp>
#include <chrono>
#include <sstream>

namespace proast { namespace presenter { 

    enum class DisplayCost
    {
        My, Total, Todo, Done,
    };
    std::string hr(DisplayCost);

    class Presenter: public model::Events, public view::Events, public Commander::Events
    {
    public:
        Presenter(model::Model &m, view::View &v): model_(m), view_(v)
        {
            model_.set_events_dst(this);
            view_.set_events_dst(this);
            commander_.set_events_dst(this);
        }
        virtual ~Presenter() {}

        bool operator()()
        {
            MSS_BEGIN(bool);

            bool do_repaint = false;

            //Repaint from time to time
            {
                const auto now = Clock::now();
                if (now >= repaint_tp_)
                {
                    do_repaint = true;
                    repaint_tp_ = now+std::chrono::milliseconds(300);
                }
            }
            //Always repaint  when commander is waiting for input
            if (commander_.waits_for_input())
                do_repaint = true;

            if (do_repaint)
                MSS(repaint_());

            MSS_END();
        }

        bool quit = false;

        //model::Events API
        void model_message(const std::string &str) override
        {
            log::stream() << "Model: " << str << std::endl;
        }
        void model_notify() override
        {
            repaint_();
        }

        //view::Events API
        void view_message(const std::string &str) override
        {
            log::stream() << "View: " << str << std::endl;
        }
        void view_received(const char32_t ch) override
        {
            commander_.process(ch);
        }

        //Commander API
        bool commander_quit() override
        {
            MSS_BEGIN(bool);
            message_("Received quit signal");
            quit = true;
            MSS_END();
        }
        bool commander_switch_mode(Mode from, Mode to) override
        {
            MSS_BEGIN(bool);

            switch (to)
            {
                case Mode::Normal:
                    switch (from)
                    {
                        case Mode::Init:
                            break;
                        case Mode::SelectLink:
                            {
                                const auto link_path = model_.path();
                                model_.set_path(normal_path_);
                                MSS(model_.add_link(link_path));
                            }
                            break;
                    }
                    break;

                case Mode::SelectLink:
                    normal_path_ = model_.path();
                    break;
            }

            mode_ = to;

            MSS_END();
        }
        bool commander_move_cursor(Movement movement) override
        {
            MSS_BEGIN(bool);

            auto path = model_.path();

            const model::Forest *forest;
            std::size_t ix;
            MSS(model_.get(forest, ix, path));

            auto set_new_ix = [&](std::size_t new_ix)
            {
                MSS_BEGIN(bool);
                path.back() = forest->nodes[new_ix].value.key;

                model::Node *parent;
                if (model_.get_parent(parent, path))
                    parent->value.active_child_key = path.back();
                MSS_END();
            };

            switch (movement)
            {
                case Movement::Left:
                    if (!path.empty())
                        path.pop_back();
                    break;
                case Movement::Right:
                    {
                        const auto &me = forest->nodes[ix];
                        const auto &childs = me.childs;
                        if (childs.empty())
                        {
                            if (me.value.content_fp || me.value.link)
                                return commander_open(true);
                        }
                        else
                        {
                            std::string active_child_key;
                            if (active_child_key.empty())
                            {
                                if (me.find_child_ix([&](const auto &node){return node.value.key == me.value.active_child_key;}))
                                    active_child_key = me.value.active_child_key;
                            }
                            if (active_child_key.empty())
                            {
                                if (!me.childs.empty())
                                    active_child_key = me.childs.nodes[0].value.key;
                            }
                            path.push_back(active_child_key);
                        }
                    }
                    break;
                case Movement::Up:
                    if (ix > 0 && !path.empty())
                        set_new_ix(ix-1);
                    break;
                case Movement::Down:
                    if (ix+1 < forest->size() && !path.empty())
                        set_new_ix(ix+1);
                    break;
                case Movement::Top:
                    set_new_ix(0);
                    break;
                case Movement::Bottom:
                    set_new_ix(forest->empty() ? 0 : forest->size()-1);
                    break;
            }

            model_.set_path(path);

            MSS(repaint_());

            MSS_END();
        }
        bool commander_move_item(Movement movement)
        {
            MSS_BEGIN(bool);

            const model::Forest *forest;
            std::size_t ix;
            MSS(model_.get(forest, ix, model_.path()));

            switch (movement)
            {
                case Movement::Left:
                    break;
                case Movement::Up:
                    if (ix > 0)
                        MSS(model_.swap(ix, ix-1));
                    break;
                case Movement::Down:
                    if (ix+1 < forest->size())
                        MSS(model_.swap(ix, ix+1));
                    break;
            }
            MSS_END();
        }
        bool commander_open(bool edit) override
        {
            MSS_BEGIN(bool);

            model::ConstNodeIXPath cnixpath;
            MSS(model_.get(cnixpath, model_.path()));

            const auto me_node = cnixpath.back().node;
            cnixpath.pop_back();

            if (me_node->value.link)
            {
                model_.set_path(*me_node->value.link);
                return true;
            }

            auto content_fp = me_node->value.content_fp;
            {
                if (!content_fp)
                    if (!cnixpath.empty())
                    {
                        const auto parent_node = cnixpath.back().node;
                        cnixpath.pop_back();
                        content_fp = parent_node->value.content_fp;
                    }
                MSS(!!content_fp, log::stream() << "Me nor parent have a valid content_fp" << std::endl);
            }

            view_.pause([&](){
                    const auto orig_dir = std::filesystem::current_path();
                    std::filesystem::current_path(content_fp->parent_path());

                    std::ostringstream oss;
                    oss << (edit ? "nvim " : "gedit ") << content_fp->filename();
                    std::system(oss.str().c_str());

                    std::filesystem::current_path(orig_dir);
                    });

            MSS(repaint_());

            MSS_END();
        }
        bool commander_add(const std::string &str, bool insert, bool is_final) override
        {
            MSS_BEGIN(bool);
            if (is_final)
            {
                dialog_.reset();
                if (!str.empty())
                    model_.add_item(str, insert);
            }
            else
            {
                if (!dialog_)
                {
                    dialog_.emplace();
                    dialog_->set_caption(insert ? "Insert new item" : "Add new item");
                }
                dialog_->set_content(str);
            }
            MSS_END();
        }
        bool commander_rename(const std::string &str, bool is_final) override
        {
            MSS_BEGIN(bool);
            if (is_final)
            {
                dialog_.reset();
                if (!str.empty())
                {
                    MSS(model_.rename_item(str), log::stream() << "Warning: Could not rename" << std::endl);
                }
            }
            else
            {
                if (!dialog_)
                {
                    dialog_.emplace();
                    dialog_->set_caption("Rename item");
                }
                dialog_->set_content(str);
            }
            MSS_END();
        }
        bool commander_cost(const std::string &str, bool new_cost, bool is_final) override
        {
            MSS_BEGIN(bool);
            if (is_final)
            {
                dialog_.reset();
                if (new_cost)
                    MSS(model_.set_cost(str), log::stream() << "Warning: Could not set cost" << std::endl);
                else
                {
                    if (false) {}
                    else if (str == "my") {display_cost_ = DisplayCost::My;}
                    else if (str == "total") {display_cost_ = DisplayCost::Total;}
                    else if (str == "done") {display_cost_ = DisplayCost::Done;}
                    else if (str == "todo") {display_cost_ = DisplayCost::Todo;}
                    else if (str == "") {display_cost_.reset();}
                }
            }
            else
            {
                if (!dialog_)
                {
                    dialog_.emplace();
                    if (new_cost)
                        dialog_->set_caption(std::string("Provide cost of item in ")+model_.current_config().cost_unit());
                    else
                        dialog_->set_caption("Specify the cost to display");
                }
                dialog_->set_content(str);
            }
            MSS_END();
        }
        bool commander_command(const std::string &str, bool is_final) override
        {
            MSS_BEGIN(bool);
            if (is_final)
            {
                dialog_.reset();

                auto is = [&](const std::string &needle){return str == needle;};
                if (is("sort"))
                {
                    if (model_.path().size() <= 1)
                    {
                        //TODO: Indicate to the user that sorting the first level cannot be done
                    }
                    else
                    {
                        MSS(model_.sort());
                    }
                }
                else if (is("export"))
                {
                    MSS(model_.export_nodes("/tmp/proast-nodes.tsv"));
                }
                else if (is("help"))
                {
                    dialog_.emplace();
                    dialog_->set_caption("Help");
                    dialog_->set_content("me");
                }
            }
            else
            {
                if (!dialog_)
                {
                    dialog_.emplace();
                    dialog_->set_caption("Command:");
                }
                dialog_->set_content(str);
            }
            MSS_END();
        }
        bool commander_remove(model::Removable removable) override
        {
            MSS_BEGIN(bool);
            MSS(model_.remove_current(removable));
            MSS_END();
        }
        bool commander_register_bookmark(char32_t ch) override
        {
            MSS_BEGIN(bool);
            MSS(model_.register_bookmark(ch));
            MSS_END();
        }
        bool commander_load_bookmark(char32_t ch) override
        {
            MSS_BEGIN(bool);
            MSS_Q(model_.load_bookmark(ch));
            MSS_END();
        }
        bool commander_set_type(char32_t ch) override
        {
            MSS_BEGIN(bool);
            std::optional<model::Type> type;
            switch (ch)
            {
                case 'r': type = model::Type::Requirement; break;
                case 'd': type = model::Type::Design; break;
                case 'f': type = model::Type::Feature; break;
                case '?': break;
                default: return true; break;
            }
            MSS(model_.set_type(type));
            MSS_END();
        }
        bool commander_set_state(char32_t ch) override
        {
            MSS_BEGIN(bool);
            std::optional<model::State> state;
            switch (ch)
            {
                case 'u':
                case '?':
                    state = model::State::Unclear; break;
                case 'c':
                case '#':
                    state = model::State::Clear; break;
                case 't':
                case '~':
                    state = model::State::Thinking; break;
                case 'D':
                case '$':
                    state = model::State::Designed; break;
                case 'i':
                case '@':
                    state = model::State::Implementing; break;
                case 'd':
                case '.':
                    state = model::State::Done; break;
                case 'n':
                case ' ':
                    break;
                default: return true; break;
            }
            MSS(model_.set_state(state));
            MSS_END();
        }
        bool commander_open_directory(bool with_shell) override
        {
            MSS_BEGIN(bool);

            model::ConstNodeIXPath cnixpath;
            MSS(model_.get(cnixpath, model_.path()));

            const auto me_node = cnixpath.back().node;
            cnixpath.pop_back();

            auto directory = me_node->value.directory;
            {
                if (!directory || !std::filesystem::exists(*directory))
                    if (!cnixpath.empty())
                    {
                        const auto parent_node = cnixpath.back().node;
                        cnixpath.pop_back();
                        directory = parent_node->value.directory;
                    }
                MSS(!!directory && std::filesystem::exists(*directory), log::stream() << "Me nor parent have a valid directory" << std::endl);
            }

            view_.pause([&](){
                    const auto orig_dir = std::filesystem::current_path();
                    std::filesystem::current_path(*directory);

                    std::system(with_shell ? "bash" : "ranger");

                    std::filesystem::current_path(orig_dir);
                    });

            MSS_END();
        }
        bool commander_paste(bool insert) override
        {
            MSS_BEGIN(bool);
            MSS(model_.paste(insert));
            MSS_END();
        }

    private:
        bool repaint_()
        {
            MSS_BEGIN(bool);

            status_ = "Starting";

            //Location
            {
                location_.clear();
                switch (mode_)
                {
                    case Mode::Normal:
                        location_ += model::to_string(model_.path());
                        break;
                    case Mode::SelectLink:
                        location_ += "Selecting link for ";
                        location_ += model::to_string(normal_path_);
                        location_ += ": ";
                        location_ += model::to_string(model_.path());
                        break;
                }
            }

            auto fill_lb = [&](auto &lb, auto forest, std::size_t ix)
            {
                lb.clear();
                if (!forest)
                    return ;
                for (const auto &node: forest->nodes)
                {
                    const auto &item = node.value;
                    unsigned int attention = 0;
                    if (item.type)
                        switch (*item.type)
                        {
                            case model::Type::Requirement: attention = 3; break;
                            case model::Type::Design: attention = 1; break;
                            case model::Type::Feature: attention = 2; break;
                            case model::Type::Free: attention = 5; break;
                            default: break;
                        }

                    std::string cost;
                    if (display_cost_)
                    {
                        const int my_cost = item.my_cost.value_or(0);
                        const int total_cost = item.total_cost;
                        const int done_cost = item.done_cost;
                        switch (*display_cost_)
                        {
                            case DisplayCost::My:    cost = std::to_string(my_cost); break;
                            case DisplayCost::Total: cost = std::to_string(total_cost); break;
                            case DisplayCost::Done:  cost = std::to_string(done_cost)+"/"+std::to_string(total_cost); break;
                            case DisplayCost::Todo:  cost = std::to_string(total_cost-done_cost)+"/"+std::to_string(total_cost); break;
                        }
                    }

                    std::string prefix = "  ";
                    if (item.state)
                        switch (*item.state)
                        {
                            case model::State::Unclear: prefix = "? "; break;
                            case model::State::Clear: prefix = "# "; break;
                            case model::State::Thinking: prefix = "~ "; break;
                            case model::State::Designed: prefix = "$ "; break;
                            case model::State::Implementing: prefix = "@ "; break;
                            case model::State::Done: prefix = ". "; break;
                        }

                    lb.entries.emplace_back(prefix+item.key, cost, attention);
                    {
                        const int total_cost = item.total_cost*10;
                        const int done_cost = item.done_cost*10;
                        lb.entries.back().done = (total_cost > 0 && total_cost == done_cost);
                    }
                }
                lb.active_ix = ix;
            };

            auto path = model_.path();
            //Me and childs
            {
                const model::Forest *me_forest = nullptr;
                std::size_t me_ix;
                for (; !path.empty(); path.pop_back())
                {
                    const auto ok = model_.get(me_forest, me_ix, path);
                    if (ok)
                        break;
                    log::stream() << "Warning: could not get me and childs for path " << model::to_string(path) << std::endl;
                }
                MSS(!path.empty(), log::stream() << "Error: the complete path is invalid" << std::endl);
                fill_lb(me_lb_, me_forest, me_ix);

                const auto &me = me_forest->nodes[me_ix];
                const auto &item = me.value;
                auto child_ix = me.find_child_ix([&](const auto &node){return node.value.key == item.active_child_key;});
                if (!child_ix)
                    child_ix = me.find_child_ix([](const auto &node){return true;});
                const auto childs_forest = (child_ix ? &me.childs : nullptr);
                fill_lb(child_lb_, childs_forest, *child_ix);

                if (Clock::now() >= show_path_)
                {
                    status_.clear();
                    status_ += "Commander state: ";
                    status_ += hr(commander_.state());
                    if (display_cost_)
                    {
                        status_ += ", ";
                        status_ += "Display cost: ";
                        status_ += hr(*display_cost_);
                    }
                }

                {
                    preview_mu_.clear();
                    gubg::markup::Style style;
                    style.attention = 1;
                    preview_mu_.add_line([&](auto &line){line.add(item.title, style);});
                    style.attention = 0;
                    preview_mu_.add_line([&](auto &line){line.add("", style);});
                    for (const auto &desc: item.description)
                        preview_mu_.add_line([&](auto &line){line.add(desc, style);});
                }

                details_kv_.clear();
                if (item.type)
                    details_kv_["type"] = model::hr(*item.type);
                details_kv_["title"] = item.title;
                if (item.link)
                    details_kv_["link"] = model::to_string(*item.link);
                if (item.state)
                    details_kv_["state"] = model::hr(*item.state);
                if (item.deadline)
                    details_kv_["deadline"] = *item.deadline;
                if (item.my_cost)
                    details_kv_["my_cost"] = std::to_string(*item.my_cost)+model_.current_config().cost_unit();
                details_kv_["total_cost"] = std::to_string(item.total_cost)+model_.current_config().cost_unit();
                details_kv_["done_cost"] = std::to_string(item.done_cost)+model_.current_config().cost_unit();
                details_kv_["todo_cost"] = std::to_string(item.total_cost-item.done_cost)+model_.current_config().cost_unit();
                if (item.directory)
                    details_kv_["directory"] = item.directory->string();
                if (item.content_fp)
                    details_kv_["content_fp"] = item.content_fp->string();
            }

            //Parent
            {
                const model::Forest *forest = nullptr;
                std::size_t ix;

                if (path.size() > 1)
                {
                    path.pop_back();

                    MSS(model_.get(forest, ix, path), log::stream() << "Error: could not get parent\n");
                }

                fill_lb(parent_lb_, forest, ix);
            }

            {
                view_.clear_screen();

                view_.show_location(location_);

                view_.show_status(status_);

                view_.show_parent(parent_lb_);
                view_.show_me(me_lb_);
                view_.show_child(child_lb_);

                view_.show_preview(preview_mu_);
                view_.show_details(details_kv_);

                if (dialog_)
                    view_.show_dialog(*dialog_);

                view_.render_screen();
            }

            MSS_END();
        }
        void message_(const std::string &str) const
        {
            log::stream() << "Presenter: " << str << std::endl;
        }

        model::Model &model_;
        view::View &view_;

        Commander commander_;
        Mode mode_ = Mode::Init;
        model::Path normal_path_;
        std::optional<DisplayCost> display_cost_;

        std::string location_;
        std::string status_;
        ListBox parent_lb_;
        ListBox me_lb_;
        ListBox child_lb_;
        gubg::markup::Document preview_mu_;
        std::map<std::string, std::string> details_kv_;
        std::optional<Dialog> dialog_;

        using Clock = std::chrono::high_resolution_clock;
        Clock::time_point repaint_tp_ = Clock::now();
        Clock::time_point show_path_ = Clock::now() + std::chrono::seconds(2);
    };

} } 

#endif
