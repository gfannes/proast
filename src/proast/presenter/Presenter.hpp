#ifndef HEADER_proast_presenter_Presenter_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_Presenter_hpp_ALREADY_INCLUDED

#include <proast/model/Model.hpp>
#include <proast/view/View.hpp>
#include <gubg/mss.hpp>
#include <chrono>

namespace proast { namespace presenter { 

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

                case 'i': model_.set_mode("input"); break;
                case 's': model_.set_mode("system"); break;
                case 'f': model_.set_mode("features"); break;
                case 'p': model_.set_mode("planning"); break;
                case 'r': model_.set_mode("report"); break;
            }
        }

    private:
        void repaint_()
        {
            std::size_t ix;
            for (ix = 0u; ix < mode_line_.size(); ++ix)
                if (mode_line_[ix] == model_.mode)
                    break;
            view_.show_mode_line(mode_line_, ix);
        }
        void message_(const std::string &str) const
        {
            log::stream() << "Presenter: " << str << std::endl;
        }

        model::Model &model_;
        view::View &view_;
        const std::vector<std::string> mode_line_ = {"input", "system", "features", "planning", "report"};

        using Clock = std::chrono::high_resolution_clock;
        Clock::time_point repaint_tp_ = Clock::now();
    };

} } 

#endif
