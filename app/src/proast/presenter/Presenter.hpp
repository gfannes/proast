#ifndef HEADER_proast_presenter_Presenter_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_Presenter_hpp_ALREADY_INCLUDED

#include <proast/presenter/ListBox.hpp>
#include <proast/model/Model.hpp>
#include <proast/view/View.hpp>
#include <gubg/mss.hpp>
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

    class Presenter: public model::Events, public view::Events
    {
    public:
        Presenter(model::Model &m, view::View &v): model_(m), view_(v)
        {
            model_.set_events_dst(this);
            view_.set_events_dst(this);
        }
        virtual ~Presenter() {}

        bool operator()()
        {
            MSS_BEGIN(bool);

            const auto now = Clock::now();
            if (now >= repaint_tp_)
            {
                //Repaint from time to time
                repaint_();
                repaint_tp_ = now+std::chrono::milliseconds(1000);
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
            switch (ch)
            {
                case 'q':
                    message_("Received quit signal");
                    quit = true;
                    break;

                case 'd': model_.set_mode(model::Mode::Develop); break;
                case 'r': model_.set_mode(model::Mode::Rework); break;
            }
        }

    private:
        void repaint_()
        {
            if (mode_lb_.items.empty())
                for (auto m = 0u; m < (unsigned int)model::Mode::Nr_; ++m)
                {
                    const auto mode_cstr = to_string((model::Mode)m);
                    if (!!mode_cstr)
                        mode_lb_.items.emplace_back(mode_cstr);
                }
            mode_lb_.set_active((int)model_.mode);
            view_.show_mode(mode_lb_);
        }
        void message_(const std::string &str) const
        {
            log::stream() << "Presenter: " << str << std::endl;
        }

        model::Model &model_;
        view::View &view_;

        ListBox mode_lb_;

        using Clock = std::chrono::high_resolution_clock;
        Clock::time_point repaint_tp_ = Clock::now();
    };

} } 

#endif
