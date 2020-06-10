#ifndef HEADER_proast_presenter_Commander_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_Commander_hpp_ALREADY_INCLUDED

#include <proast/model/Model.hpp>
#include <gubg/mss.hpp>

namespace proast { namespace presenter { 

    enum class Movement
    {
        Left, Down, Up, Right, Top, Bottom,
    };
    enum class Mode
    {
        Init, Normal, SelectLink,
    };
    enum class State
    {
        Idle, Add, Rename, Remove, Cost, RegisterBookmark, LoadBookmark, SetType, SetState, Command,
    };
    std::string hr(State state);

    //Translates key-presses into commands

    class Commander
    {
    public:
        //Events produced by this commander
        class Events
        {
        public:
            virtual bool commander_quit() = 0;
            virtual bool commander_switch_mode(Mode from, Mode to) = 0;
            virtual bool commander_move_cursor(Movement) = 0;
            virtual bool commander_move_item(Movement) = 0;
            virtual bool commander_open() = 0;
            virtual bool commander_add(const std::string &str, bool insert, bool is_final) = 0;
            virtual bool commander_rename(const std::string &str, bool is_final) = 0;
            virtual bool commander_cost(const std::string &str, bool new_cost, bool is_final) = 0;
            virtual bool commander_remove(model::Removable) = 0;
            virtual bool commander_register_bookmark(char32_t) = 0;
            virtual bool commander_load_bookmark(char32_t) = 0;
            virtual bool commander_set_type(char32_t) = 0;
            virtual bool commander_set_state(char32_t) = 0;
            virtual bool commander_command(const std::string &) = 0;
            virtual bool commander_open_directory(bool with_shell) = 0;
            virtual bool commander_paste(bool insert) = 0;
        };

        //Set the events listener
        void set_events_dst(Events *events)
        {
            events_ = events;
            switch_mode_(Mode::Normal);
        }

        State state() const {return state_;}

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

                                  //Movement of cursor
                        case 'h':
                        case 0x82:
                                  MSS(events_->commander_move_cursor(Movement::Left)); break;
                        case 'j':
                        case 0x81:
                                  MSS(events_->commander_move_cursor(Movement::Down)); break;
                        case 'k':
                        case 0x80:
                                  MSS(events_->commander_move_cursor(Movement::Up)); break;
                        case 'l':
                        case 0x83:
                                  MSS(events_->commander_move_cursor(Movement::Right)); break;
                        case 'g': MSS(events_->commander_move_cursor(Movement::Top)); break;
                        case 'G': MSS(events_->commander_move_cursor(Movement::Bottom)); break;

                                  //Movement of item
                        case 'H':
                                  MSS(events_->commander_move_item(Movement::Left)); break;
                        case 'J':
                                  MSS(events_->commander_move_item(Movement::Down)); break;
                        case 'K':
                                  MSS(events_->commander_move_item(Movement::Up)); break;

                        case '\n':
                                  switch (mode_)
                                  {
                                      case Mode::Normal:
                                          MSS(events_->commander_open());
                                          break;
                                      case Mode::SelectLink:
                                          MSS(switch_mode_(Mode::Normal));
                                          break;
                                  }
                                  break;

                                   //Add item
                        case 'O': add_insert_ = true;  change_state_(State::Add); break;
                        case 'o': add_insert_ = false; change_state_(State::Add); break;

                                  //Add link
                        case 'a': MSS(switch_mode_(Mode::SelectLink)); break;

                                  //Rename item
                        case 'r': change_state_(State::Rename); break;

                                  //Remove item
                        case 'd': change_state_(State::Remove); break;

                                  //Paste
                        case 'P': MSS(events_->commander_paste(true)); break;
                        case 'p': MSS(events_->commander_paste(false)); break;

                                  //Cost
                        case 'c': new_cost_ = true;  change_state_(State::Cost); break;
                        case 'C': new_cost_ = false; change_state_(State::Cost); break;

                                  //Bookmarks
                        case 'm': change_state_(State::RegisterBookmark); break;
                        case '\'': change_state_(State::LoadBookmark); break;

                                  //State
                        case 't': change_state_(State::SetState); break;

                                  //Type
                        case 'T': change_state_(State::SetType); break;

                                  //Open shell/browser
                        case 's': MSS(events_->commander_open_directory(false)); break;
                        case 'S': MSS(events_->commander_open_directory(true)); break;

                                  //Command
                        case ':': change_state_(State::Command); break;
                    }
                    break;
                case State::Add:
                case State::Rename:
                case State::Cost:
                case State::Command:
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
                        case 'd': removable_ = model::Removable::Node; change_state_(State::Idle); break;
                        case 'D': removable_ = model::Removable::File;  change_state_(State::Idle); break;
                        case '!': removable_ = model::Removable::Folder;  change_state_(State::Idle); break;
                        default:
                                  removable_.reset();
                                  change_state_(State::Idle);
                                  break;
                    }
                    break;
                case State::RegisterBookmark:
                case State::LoadBookmark:
                    switch (ch)
                    {
                        case 0x1B://Escape
                            change_state_(State::Idle);
                            break;
                        default:
                            bookmark_cb_(ch);
                            change_state_(State::Idle);
                            break;
                    }
                    break;
                case State::SetType:
                    switch (ch)
                    {
                        case 0x1B://Escape
                            change_state_(State::Idle);
                            break;
                        default:
                            if (events_ && events_->commander_set_type(ch))
                                change_state_(State::Idle);
                            break;
                    }
                    break;
                case State::SetState:
                    switch (ch)
                    {
                        case 0x1B://Escape
                            change_state_(State::Idle);
                            break;
                        default:
                            if (events_ && events_->commander_set_state(ch))
                                change_state_(State::Idle);
                            break;
                    }
                    break;
            }
            MSS_END();
        }

        bool switch_mode_(Mode mode)
        {
            MSS_BEGIN(bool);
            if (events_)
                MSS(events_->commander_switch_mode(mode_, mode));
            mode_ = mode;
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
                case State::Command:
                    user_input_cb_(true);
                    user_input_cb_ = nullptr;
                    break;
                case State::Remove:
                    if (removable_ && events_)
                        events_->commander_remove(*removable_);
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
                case State::Command:
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
                                    events_->commander_cost(user_input_, new_cost_, is_final);
                            };
                            break;
                        case State::Command:
                            user_input_cb_ = [&](bool is_final)
                            {
                                if (!is_final)
                                    return;
                                if (!events_)
                                    return;
                                events_->commander_command(user_input_);
                            };
                            break;
                    }
                    user_input_cb_(false);
                    break;
                case State::Remove:
                    removable_.reset();
                    break;
                case State::RegisterBookmark:
                    bookmark_cb_ = [&](char32_t ch)
                    {
                        if (events_)
                            events_->commander_register_bookmark(ch);
                    };
                    break;
                case State::LoadBookmark:
                    bookmark_cb_ = [&](char32_t ch)
                    {
                        if (events_)
                            events_->commander_load_bookmark(ch);
                    };
                    break;
                default:
                    break;
            }
        }

        Events *events_ = nullptr;
        State state_ = State::Idle;
        Mode mode_ = Mode::Init;
        std::string user_input_;
        bool add_insert_ = false;
        std::function<void(bool)> user_input_cb_;
        std::optional<model::Removable> removable_;
        bool new_cost_ = false;

        std::function<void(char32_t)> bookmark_cb_;
    };

} } 

#endif
