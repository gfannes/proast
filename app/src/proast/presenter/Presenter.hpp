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
        void commander_quit() override
        {
            message_("Received quit signal");
            quit = true;
        }
        void commander_set_mode(model::Mode mode) override
        {
            model_.set_mode(mode);
        }

    private:
        bool repaint_()
        {
            MSS_BEGIN(bool);
            if (mode_lb_.items.empty())
                for (auto m = 0u; m < (unsigned int)model::Mode::Nr_; ++m)
                {
                    const auto mode_cstr = to_string((model::Mode)m);
                    if (!!mode_cstr)
                        mode_lb_.items.emplace_back(mode_cstr);
                }
            mode_lb_.set_active((int)model_.mode);

            view_.clear_screen();
            view_.show_mode(mode_lb_);
            view_.show_status(std::string("root path: ")+model_.root_path().string());
            view_.show_parent(parent_lb_);

            const model::Forest *forest = nullptr;
            std::size_t ix;

            auto fill_lb = [&](auto &lb, auto forest, std::size_t ix)
            {
                if (!forest)
                    return ;
                lb.clear();
                for (const auto &node: forest->nodes)
                    lb.items.push_back(node.value.short_name);
                lb.active_ix = ix;
            };

            {
                MSS(model_.get_me(forest, ix));
                fill_lb(me_lb_, forest, ix);
            }

            {
                const auto &me = forest->nodes[ix];
                forest = (ix < me.nr_childs() ? &me.childs : nullptr);
                ix = me.value.active_ix;
                fill_lb(child_lb_, forest, ix);
            }

            view_.show_me(me_lb_);
            view_.show_child(child_lb_);

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
