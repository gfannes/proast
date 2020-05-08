#ifndef HEADER_proast_presenter_Presenter_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_Presenter_hpp_ALREADY_INCLUDED

#include <proast/presenter/Commander.hpp>
#include <proast/presenter/ListBox.hpp>
#include <proast/model/Model.hpp>
#include <proast/view/View.hpp>
#include <gubg/mss.hpp>
#include <gubg/Range.hpp>
#include <chrono>

namespace proast { namespace presenter { 

    inline const char *to_string(model::Mode m)
    {
        switch (m)
        {
            case model::Mode::Develop: return "develop"; break;
            case model::Mode::Rework: return "rework"; break;
            default: break;
        }
        return 0;
    }

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

            const auto now = Clock::now();
            if (now >= repaint_tp_)
            {
                //Repaint from time to time
                MSS(repaint_());
                repaint_tp_ = now+std::chrono::milliseconds(300);
            }

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
        bool commander_set_mode(model::Mode mode) override
        {
            MSS_BEGIN(bool);
            model_.set_mode(mode);
            MSS_END();
        }
        bool commander_move(Movement movement) override
        {
            MSS_BEGIN(bool);

            auto path = model_.path();

            const model::Forest *forest;
            std::size_t ix;
            MSS(model_.get(forest, ix, path));

            switch (movement)
            {
                case Movement::Left:
                    if (!path.empty())
                        path.pop_back();
                    break;
                case Movement::Right:
                    {
                        const auto &me = forest->nodes[ix];
                        const auto child_ix = me.value.active_ix;
                        if (child_ix < me.nr_childs())
                        {
                            const auto &child = me.childs.nodes[child_ix];
                            path.push_back(child.value.short_name);
                        }
                    }
                    break;
                case Movement::Up:
                    if (ix > 0 && !path.empty())
                        path.back() = forest->nodes[ix-1].value.short_name;
                    break;
                case Movement::Down:
                    if (ix+1 < forest->size() && !path.empty())
                        path.back() = forest->nodes[ix+1].value.short_name;
                    break;
            }

            model_.set_path(path);

            MSS(repaint_());

            MSS_END();
        }

    private:
        bool repaint_()
        {
            MSS_BEGIN(bool);

            view_.clear_screen();

            if (mode_lb_.items.empty())
                for (auto m = 0u; m < (unsigned int)model::Mode::Nr_; ++m)
                {
                    const auto mode_cstr = to_string((model::Mode)m);
                    if (!!mode_cstr)
                        mode_lb_.items.emplace_back(mode_cstr);
                }
            mode_lb_.set_active((int)model_.mode);

            if (true)
            {
                view_.show_path(model_.path());
            }
            else
            {
                view_.show_mode(mode_lb_);
            }
            view_.show_status(std::string("root path: ")+model_.root_filepath().string());

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
                const model::Forest *forest = nullptr;
                std::size_t ix;
                MSS(model_.get(forest, ix, path), log::stream() << "Error: could not get me and childs\n");
                fill_lb(me_lb_, forest, ix);

                const auto &me = forest->nodes[ix];
                forest = (ix < me.nr_childs() ? &me.childs : nullptr);
                ix = me.value.active_ix;
                fill_lb(child_lb_, forest, ix);
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

            view_.show_parent(parent_lb_);
            view_.show_me(me_lb_);
            view_.show_child(child_lb_);

            view_.render_screen();

            MSS_END();
        }
        void message_(const std::string &str) const
        {
            log::stream() << "Presenter: " << str << std::endl;
        }

        model::Model &model_;
        view::View &view_;

        Commander commander_;

        ListBox mode_lb_;
        ListBox parent_lb_;
        ListBox me_lb_;
        ListBox child_lb_;

        using Clock = std::chrono::high_resolution_clock;
        Clock::time_point repaint_tp_ = Clock::now();
    };

} } 

#endif
