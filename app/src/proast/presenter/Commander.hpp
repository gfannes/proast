#ifndef HEADER_proast_presenter_Commander_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_Commander_hpp_ALREADY_INCLUDED

#include <proast/model/Model.hpp>
#include <gubg/mss.hpp>

namespace proast { namespace presenter { 

    //Translates key-presses into commands

    class Commander
    {
    public:
        //Events produced by this commander
        class Events
        {
        public:
            virtual void commander_quit() = 0;
            virtual void commander_set_mode(model::Mode) = 0;
        };

        //Set the events listener
        void set_events_dst(Events *events) {events_ = events;}

        void process(const char32_t ch)
        {
            if (!process_(ch))
                if (events_)
                    events_->commander_quit();
        }

    private:
        bool process_(const char32_t ch)
        {
            MSS_BEGIN(bool);
            MSS(!!events_);
            switch (ch)
            {
                case 'q': events_->commander_quit(); break;
                case 'd': events_->commander_set_mode(model::Mode::Develop); break;
                case 'r': events_->commander_set_mode(model::Mode::Rework); break;
            }
            MSS_END();
        }

        Events *events_ = nullptr;
    };

} } 

#endif
