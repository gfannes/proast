#ifndef HEADER_proast_presenter_Commander_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_Commander_hpp_ALREADY_INCLUDED

namespace proast { namespace presenter { 
    enum class Direction
    {
        Down, Up, Left, Right,
    };

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
                case L'k': r.commander_move(Direction::Up); break;
                case L'h': r.commander_move(Direction::Left); break;
                case L'l': r.commander_move(Direction::Right); break;
            }
        }

    private:
        Receiver &receiver_() {return *static_cast<Receiver*>(this);}
    };
} } 

#endif
