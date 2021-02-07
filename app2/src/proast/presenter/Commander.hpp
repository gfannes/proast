#ifndef HEADER_proast_presenter_Commander_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_Commander_hpp_ALREADY_INCLUDED

#include <proast/presenter/enums.hpp>
#include <proast/types.hpp>
#include <proast/unicode.hpp>
#include <string>
#include <optional>

namespace proast { namespace presenter { 

    inline std::optional<MetadataField> to_metadata_field(char ch)
    {
        switch (ch)
        {
            case 'e': return MetadataField::Effort;
            case 'v': return MetadataField::Volume;
            case 'i': return MetadataField::Impact;
            case 'c': return MetadataField::CompletionPct;
            case 'l': return MetadataField::Live;
            case 'd': return MetadataField::Dead;
            case 't': return MetadataField::Tag;
            case 'D': return MetadataField::Dependency;
            default: break;
        }
        return std::nullopt;
    }

    template <typename Receiver>
    class Commander_crtp
    {
    public:
        std::optional<State> state;
        std::optional<MetadataField> metadata_field;
        std::string content;
        std::optional<char> create_what;

        void process(wchar_t wchar)
        {
            auto &r = receiver_();
            //TODO: integrate the `chain of responsibility` pattern
            //* It is currently not possible to clear `content` with Escape when setting a metadata field
            if (wchar == Escape)
            {
                state.reset();
            }
            else if (state)
            {
                switch (*state)
                {
                    case State::BookmarkRegister:
                        r.commander_bookmark(wchar, true);
                        state.reset();
                        break;
                    case State::BookmarkJump:
                        r.commander_bookmark(wchar, false);
                        state.reset();
                        break;
                    case State::SetMetadataField:
                        if (!metadata_field)
                        {
                            content.clear();
                            if (!(metadata_field = to_metadata_field(wchar)))
                                state.reset();
                        }
                        else
                            switch (wchar)
                            {
                                case Return:
                                    r.commander_set_metadata(*metadata_field, content);
                                    content.clear();
                                    metadata_field.reset();
                                    state.reset();
                                    break;
                                case Backspace:
                                    if (!content.empty())
                                        content.pop_back();
                                    break;
                                case Escape:
                                    //Does not work, requires `chain of responsibility`: Escape is handled too soon and will reset the `state`
                                    if (!content.empty())
                                        content.clear();
                                    else
                                        metadata_field.reset();
                                    break;
                                default:
                                    content.push_back(wchar);
                                    break;
                            }
                        break;
                    case State::ShowMetadataField:
                        r.commander_show_metadata(to_metadata_field(wchar));
                        state.reset();
                        break;
                    case State::Create:
                        if (!create_what)
                        {
                            switch (wchar)
                            {
                                case 'f':
                                case 'F':
                                case 'd':
                                case 'D':
                                    create_what = wchar;
                                    break;
                                default:
                                    break;
                            }
                        }
                        else
                            switch (wchar)
                            {
                                case Return:
                                    {
                                        const auto ch = *create_what;
                                        r.commander_create(content, ch=='f'||ch=='F', ch=='f'||ch=='d');
                                        content.clear();
                                        state.reset();
                                        create_what.reset();
                                    }
                                    break;
                                case Backspace:
                                    if (!content.empty())
                                        content.pop_back();
                                    break;
                                case Escape:
                                    //Does not work, requires `chain of responsibility`: Escape is handled too soon and will reset the `state`
                                    content.clear();
                                    break;
                                default:
                                    content.push_back(wchar);
                                    break;
                            }
                        break;
                    case State::Delete:
                        switch (wchar)
                        {
                            case 'd': r.commander_delete(); break;
                            default: break;
                        }
                        state.reset();
                        break;
                }
            }
            else
            {
                switch (wchar)
                {
                    case 'q': r.commander_quit(); break;

                    case 'j':
                    case ArrowDown: r.commander_move(Direction::Down, true); break;

                    case 'J': r.commander_move(Direction::Down, false); break;

                    case 'k':
                    case ArrowUp: r.commander_move(Direction::Up, true); break;

                    case 'K': r.commander_move(Direction::Up, false); break;

                    case 'h': 
                    case ArrowLeft: r.commander_move(Direction::Left, true); break;

                    case 'l':
                    case ArrowRight: r.commander_move(Direction::Right, true); break;

                    case Return: r.commander_open(Open::View); break;
                    case 'e': r.commander_open(Open::Edit); break;
                    case 'S':   r.commander_open(Open::Shell); break;

                    case 'm':  state = State::BookmarkRegister; break;
                    case '\'': state = State::BookmarkJump; break;
                    case 's':  state = State::SetMetadataField; break;
                    case 'M':  state = State::ShowMetadataField; break;
                    case 'c':  state = State::Create; break;
                    case 'd':  state = State::Delete; break;

                    case 'r': r.commander_reload(); break;
                }
            }
        }

    private:
        Receiver &receiver_() {return *static_cast<Receiver*>(this);}
    };
} } 

#endif
