#ifndef HEADER_proast_presenter_Commander_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_Commander_hpp_ALREADY_INCLUDED

#include <proast/Types.hpp>

namespace proast { namespace presenter { 
    template <typename Receiver>
    class Commander_crtp
    {
    public:
        void process(wchar_t wchar)
        {
            auto &r = receiver_();
            switch (wchar)
            {
                case L'q': r.commander_quit(); break;

                case L'j':
                case L'\u2193'://Unicode down
                           r.commander_move(Direction::Down, true); break;
                case L'J': r.commander_move(Direction::Down, false); break;
                case L'k':
                case L'\u2191'://Unicode up
                           r.commander_move(Direction::Up, true); break;
                case L'K': r.commander_move(Direction::Up, false); break;
                case L'h': 
                case L'\u2190'://Unicode left
                           r.commander_move(Direction::Left, true); break;
                case L'l':
                case L'\u2192'://Unicode right
                           r.commander_move(Direction::Right, true); break;

                case L'\n': r.commander_open(true); break;
                case L's':  r.commander_open(false); break;
            }
        }

    private:
        Receiver &receiver_() {return *static_cast<Receiver*>(this);}
    };
} } 

#endif
