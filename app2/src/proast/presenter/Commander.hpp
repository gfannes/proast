#ifndef HEADER_proast_presenter_Commander_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_Commander_hpp_ALREADY_INCLUDED

#include <proast/types.hpp>
#include <proast/unicode.hpp>
#include <string>
#include <optional>

namespace proast { namespace presenter { 
    enum class Open
    {
        View, Edit, Shell,
    };

    enum class State
    {
        Idle, BookmarkRegister, BookmarkJump,
    };

    template <typename Receiver>
    class Commander_crtp
    {
    public:
        void process(wchar_t wchar)
        {
            auto &r = receiver_();
            if (wchar == Escape)
            {
                state_.reset();
            }
            else if (state_)
            {
                switch (*state_)
                {
                    case L'm':
                        r.commander_bookmark(wchar, true);
                        state_.reset();
                        break;
                    case L'\'':
                        r.commander_bookmark(wchar, false);
                        state_.reset();
                        break;
                }
            }
            else
            {
                switch (wchar)
                {
                    case L'q': r.commander_quit(); break;

                    case L'j':
                    case ArrowDown:
                               r.commander_move(Direction::Down, true); break;
                    case L'J': r.commander_move(Direction::Down, false); break;
                    case L'k':
                    case ArrowUp:
                               r.commander_move(Direction::Up, true); break;
                    case L'K': r.commander_move(Direction::Up, false); break;
                    case L'h': 
                    case ArrowLeft:
                               r.commander_move(Direction::Left, true); break;
                    case L'l':
                    case ArrowRight:
                               r.commander_move(Direction::Right, true); break;

                    case Return: r.commander_open(Open::View); break;
                    case L'e': r.commander_open(Open::Edit); break;
                    case L's':   r.commander_open(Open::Shell); break;

                    case L'm':
                    case L'\'':
                                 state_.emplace(wchar);
                                 break;
                }
            }
        }

        State state() const
        {
            auto state = State::Idle;
            if (state_)
                switch (*state_)
                {
                    case L'm':  state = State::BookmarkRegister; break;
                    case L'\'': state = State::BookmarkJump; break;
                    default: break;
                }
            return state;
        }

    private:
        Receiver &receiver_() {return *static_cast<Receiver*>(this);}
        std::optional<wchar_t> state_;
    };
} } 

#endif
