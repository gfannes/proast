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

                case L'j': r.commander_move(Direction::Down); break;
                case L'J': r.commander_move(Direction::Down, 1); break;
                case L'k': r.commander_move(Direction::Up); break;
                case L'K': r.commander_move(Direction::Up, 1); break;
                case L'h': r.commander_move(Direction::Left); break;
                case L'l': r.commander_move(Direction::Right); break;
            }
        }

    private:
        Receiver &receiver_() {return *static_cast<Receiver*>(this);}
    };
} } 

#endif
