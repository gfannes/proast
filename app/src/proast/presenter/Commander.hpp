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
            virtual bool commander_move(Movement) = 0;
            virtual bool commander_open() = 0;
            virtual bool commander_add(const std::string &str, bool insert, bool is_final) = 0;
            virtual bool commander_rename(const std::string &str, bool is_final) = 0;
            virtual bool commander_cost(const std::string &str, bool is_final) = 0;
            virtual bool commander_remove() = 0;
        };

        //Set the events listener
        void set_events_dst(Events *events) {events_ = events;}

        bool waits_for_input() const
        {
            if (state_ == State::Idle)
                return false;
            return true;
        }

        void process(const char32_t ch)
        {
            if (!process_(ch))
                if (events_)
                    events_->commander_quit();
        }

    private:
        enum class State {Idle, Add, Rename, Remove, Cost, };

        bool process_(const char32_t ch)
        {
            MSS_BEGIN(bool);
            MSS(!!events_);

            switch (state_)
            {
                case State::Idle:
                    switch (ch)
                    {
                        case 'q': MSS(events_->commander_quit()); break;

                                  //Movement
                        case 'h': MSS(events_->commander_move(Movement::Left)); break;
                        case 'j': MSS(events_->commander_move(Movement::Down)); break;
                        case 'k': MSS(events_->commander_move(Movement::Up)); break;
                        case 'l': MSS(events_->commander_move(Movement::Right)); break;
                        case 'g': MSS(events_->commander_move(Movement::Top)); break;
                        case 'G': MSS(events_->commander_move(Movement::Bottom)); break;

                        case '\n': MSS(events_->commander_open()); break;

                                   //Add item
                        case 'i': add_insert_ = true;  change_state_(State::Add); break;
                        case 'o': add_insert_ = false; change_state_(State::Add); break;

                                  //Rename item
                        case 'r': change_state_(State::Rename); break;

                                  //Remove item
                        case 'd': change_state_(State::Remove); break;

                                  //Cost
                        case 'c': change_state_(State::Cost); break;
                    }
                    break;
                case State::Add:
                case State::Rename:
                case State::Cost:
                    switch (ch)
                    {
                        case '\n':    
                            change_state_(State::Idle);
                            break;
                        case 0x1B://Escape
                            if (user_input_.empty())
                                change_state_(State::Idle);
                            else
                            {
                                user_input_.clear();
                                user_input_cb_(false);
                            }
                            break;
                        case 0x7F://Backspace
                            if (!user_input_.empty())
                            {
                                user_input_.pop_back();
                                user_input_cb_(false);
                            }
                            break;
                        default:
                            user_input_.push_back(ch);
                            user_input_cb_(false);
                            break;
                    }
                    break;
                case State::Remove:
                    switch (ch)
                    {
                        case 'd': change_state_(State::Idle); break;
                        default:
                                  remove_ = false;
                                  change_state_(State::Idle);
                                  break;
                    }
                    break;
            }
            MSS_END();
        }

        void change_state_(State new_state)
        {
            if (new_state == state_)
                return;

            //Exit actions
            switch (state_)
            {
                case State::Add:
                case State::Rename:
                case State::Cost:
                    user_input_cb_(true);
                    user_input_cb_ = nullptr;
                    break;
                case State::Remove:
                    if (remove_ && events_)
                        events_->commander_remove();
                    break;
                default:
                    break;
            }

            state_ = new_state;

            //Entry actions
            switch (state_)
            {
                case State::Add:
                case State::Rename:
                case State::Cost:
                    user_input_.clear();
                    switch (state_)
                    {
                        case State::Add:
                            user_input_cb_ = [&](bool is_final)
                            {
                                if (events_)
                                    events_->commander_add(user_input_, add_insert_, is_final);
                            };
                            break;
                        case State::Rename:
                            user_input_cb_ = [&](bool is_final)
                            {
                                if (events_)
                                    events_->commander_rename(user_input_, is_final);
                            };
                            break;
                        case State::Cost:
                            user_input_cb_ = [&](bool is_final)
                            {
                                if (events_)
                                    events_->commander_cost(user_input_, is_final);
                            };
                            break;
                    }
                    user_input_cb_(false);
                    break;
                case State::Remove:
                    remove_ = true;
                    break;
                default:
                    break;
            }
        }

        Events *events_ = nullptr;
        State state_ = State::Idle;
        std::string user_input_;
        bool add_insert_ = false;
        std::function<void(bool)> user_input_cb_;
        bool remove_ = false;
    };

} } 

#endif
