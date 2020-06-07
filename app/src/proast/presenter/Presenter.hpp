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
        bool commander_move(Movement movement) override
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
                                return commander_open();
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
        bool commander_open() override
        {
            MSS_BEGIN(bool);

            const model::Forest *forest;
            std::size_t ix;
            MSS(model_.get(forest, ix, model_.path()));

            const auto &me = forest->nodes[ix];

            if (false) {}
            else if (me.value.content_fp)
            {
                const auto content_fp = *me.value.content_fp;
                view_.pause([&](){
                        const auto orig_dir = std::filesystem::current_path();
                        std::filesystem::current_path(content_fp.parent_path());

                        std::ostringstream oss;
                        oss << "nvim " << content_fp.filename();
                        std::system(oss.str().c_str());

                        std::filesystem::current_path(orig_dir);
                        });
            }
            else if (me.value.link)
            {
                model_.set_path(*me.value.link);
            }

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
        bool commander_cost(const std::string &str, bool is_final) override
        {
            MSS_BEGIN(bool);
            if (is_final)
            {
                dialog_.reset();
                MSS(model_.set_cost(str), log::stream() << "Warning: Could not set cost" << std::endl);
            }
            else
            {
                if (!dialog_)
                {
                    dialog_.emplace();
                    dialog_->set_caption(std::string("Provide cost of item in ")+model_.current_config().cost_unit());
                }
                dialog_->set_content(str);
            }
            MSS_END();
        }
        bool commander_remove() override
        {
            MSS_BEGIN(bool);
            MSS(model_.remove_current());
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
            }
            if (type)
                MSS(model_.set_type(*type));
            MSS_END();
        }
        bool commander_sort() override
        {
            MSS_BEGIN(bool);
            if (model_.path().size() <= 1)
            {
                //TODO: Indicate to the user that sorting the first level cannot be done
            }
            else
            {
                MSS(model_.sort());
            }
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
                    lb.items.push_back(node.value.key);
                lb.active_ix = ix;
            };

            auto path = model_.path();
            //Me and childs
            {
                const model::Forest *me_forest = nullptr;
                std::size_t me_ix;
                MSS(model_.get(me_forest, me_ix, path), log::stream() << "Error: could not get me and childs for path " << model::to_string(path) << "\n");
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
                    status_ = std::string("Commander state: ")+hr(commander_.state());
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
                details_kv_["status"] = model::hr(item.status);
                if (item.deadline)
                    details_kv_["deadline"] = *item.deadline;
                if (item.my_cost)
                    details_kv_["cost"] = std::to_string(*item.my_cost)+model_.current_config().cost_unit();
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
