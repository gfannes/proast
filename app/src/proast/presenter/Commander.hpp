#ifndef HEADER_proast_presenter_Commander_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_Commander_hpp_ALREADY_INCLUDED

#include <proast/model/Model.hpp>
#include <gubg/mss.hpp>

namespace proast { namespace presenter { 

    enum Movement
    {
        Left, Down, Up, Right, Top, Bottom,
    };

    //Translates key-presses into commands

    class Commander
    {
    public:
        //Events produced by this commander
        class Events
        {
        public:
            virtual bool commander_quit() = 0;
            virtual bool commander_set_mode(model::Mode) = 0;
            virtual bool commander_move(Movement) = 0;
            virtual bool commander_open() = 0;
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
                case 'q': MSS(events_->commander_quit()); break;

                          //Mode
                case 'd': MSS(events_->commander_set_mode(model::Mode::Develop)); break;
                case 'r': MSS(events_->commander_set_mode(model::Mode::Rework)); break;

                          //Movement
                case 'h': MSS(events_->commander_move(Movement::Left)); break;
                case 'j': MSS(events_->commander_move(Movement::Down)); break;
                case 'k': MSS(events_->commander_move(Movement::Up)); break;
                case 'l': MSS(events_->commander_move(Movement::Right)); break;
                case 'g': MSS(events_->commander_move(Movement::Top)); break;
                case 'G': MSS(events_->commander_move(Movement::Bottom)); break;

                case '\n': MSS(events_->commander_open()); break;
            }
            MSS_END();
        }

        Events *events_ = nullptr;
    };

} } 

#endif
