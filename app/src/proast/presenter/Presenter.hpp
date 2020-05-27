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
                path.back() = forest->nodes[new_ix].value.short_name;

                model::Node *parent;
                MSS(model_.get_parent(parent, path));
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
                            if (std::filesystem::is_regular_file(me.value.path))
                                return commander_open();
                        }
                        else
                        {
                            std::string active_child_key;
                            if (active_child_key.empty())
                            {
                                if (me.find_child_ix([&](const auto &node){return node.value.short_name == me.value.active_child_key;}))
                                    active_child_key = me.value.active_child_key;
                            }
                            if (active_child_key.empty())
                            {
                                if (!me.childs.empty())
                                    active_child_key = me.childs.nodes[0].value.short_name;
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

            auto path = model_.path();

            const model::Forest *forest;
            std::size_t ix;
            MSS(model_.get(forest, ix, path));

            const auto &me = forest->nodes[ix];
            if (std::filesystem::is_regular_file(me.value.path))
            {
                view_.pause([&](){
                        std::ostringstream oss;
                        oss << "nvim " << me.value.path;
                        std::system(oss.str().c_str());
                        });
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
                    model_.rename_item(str);
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
        bool commander_remove() override
        {
            MSS_BEGIN(bool);
            MSS(model_.remove_current());
            MSS_END();
        }

    private:
        bool repaint_()
        {
            MSS_BEGIN(bool);

            status_ = std::string("Loaded root path: ")+model_.root_filepath().string();

            auto fill_lb = [&](auto &lb, auto forest, std::size_t ix)
            {
                lb.clear();
                if (!forest)
                    return ;
                for (const auto &node: forest->nodes)
                    lb.items.push_back(node.value.short_name);
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
                auto child_ix = me.find_child_ix([&](const auto &node){return node.value.short_name == me.value.active_child_key;});
                if (!child_ix)
                    child_ix = me.find_child_ix([](const auto &node){return true;});
                const auto childs_forest = (child_ix ? &me.childs : nullptr);
                fill_lb(child_lb_, childs_forest, *child_ix);

                if (Clock::now() >= show_path_)
                    status_ = std::string("Current path: ") + me.value.path.string();

                preview_mu_ = me.value.preview;
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

                view_.show_path(model_.path());

                view_.show_status(status_);

                view_.show_parent(parent_lb_);
                view_.show_me(me_lb_);
                view_.show_child(child_lb_);

                view_.show_preview(preview_mu_);
                
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

        std::string status_;
        ListBox parent_lb_;
        ListBox me_lb_;
        ListBox child_lb_;
        gubg::markup::Document preview_mu_;
        std::optional<Dialog> dialog_;

        using Clock = std::chrono::high_resolution_clock;
        Clock::time_point repaint_tp_ = Clock::now();
        Clock::time_point show_path_ = Clock::now() + std::chrono::seconds(2);
    };

} } 

#endif
