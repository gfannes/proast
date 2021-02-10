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
        std::optional<bool> create_file_dir;
        std::optional<bool> create_in_next;
        std::string content;

        void process(wchar_t wchar, bool alt)
        {
            auto &r = receiver_();
            //TODO: integrate the `chain of responsibility` pattern
            //* It is currently not possible to clear `content` with Escape when setting a metadata field
            if (wchar == Escape)
            {
                state.reset();
                metadata_field.reset();
                create_file_dir.reset();
                create_in_next.reset();
                content.clear();
            }
            else if (state)
            {
                auto read_content = [&](auto &&ftor)
                {
                    switch (wchar)
                    {
                        case Return:
                            {
                                ftor();
                                content.clear();
                                state.reset();
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
                };

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
                                    metadata_field.reset();
                                    content.clear();
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
                        if (!create_file_dir || !create_in_next)
                        {
                            switch (wchar)
                            {
                                case 'f': create_file_dir = true; break;
                                case 'd': create_file_dir = false; break;
                                case 'i': create_in_next = true; break;
                                case 'n': create_in_next = false; break;
                                default: break;
                            }
                        }
                        else
                            read_content([&](){
                                    r.commander_create(content, *create_file_dir, *create_in_next);
                                    create_file_dir.reset();
                                    create_in_next.reset();
                                    });
                        break;
                    case State::Delete:
                        switch (wchar)
                        {
                            case 'd': r.commander_delete(Delete::One); state.reset(); break;
                            case 'a': r.commander_delete(Delete::Append); state.reset(); break;
                            case 'c': r.commander_delete(Delete::Clear); state.reset(); break;
                            default: break;
                        }

                        break;
                    case State::Paste:
                        switch (wchar)
                        {
                            case 'i': r.commander_paste(true);  state.reset(); break;
                            case 'n': r.commander_paste(false); state.reset(); break;
                            default: break;
                        }
                        break;
                    case State::Rename:
                        read_content([&](){r.commander_rename(content);});
                        break;
                    case State::Run:
                        read_content([&](){r.commander_open(Open::Run, content);});
                        break;
                    case State::Duplicate:
                        read_content([&](){r.commander_duplicate(content);});
                        break;
                    case State::Export:
                        read_content([&](){r.commander_export(content);});
                        break;
                }
            }
            else
            {
                switch (wchar)
                {
                    case 'q': r.commander_quit(); break;

                    case 'j':
                    case ArrowDown:  r.commander_move(Direction::Down, true, alt); break;
                    case 'J':        r.commander_move(Direction::Down, false, alt); break;

                    case 'k':
                    case ArrowUp:    r.commander_move(Direction::Up, true, alt); break;
                    case 'K':        r.commander_move(Direction::Up, false, alt); break;

                    case 'h': 
                    case ArrowLeft:  r.commander_move(Direction::Left, true, alt); break;

                    case 'l':
                    case ArrowRight: r.commander_move(Direction::Right, true, alt); break;

                    case Return: r.commander_open(Open::View); break;
                    case 'e': r.commander_open(Open::Edit); break;
                    case 'S':   r.commander_open(Open::Shell); break;
                    case 'E': state = State::Run; break;

                    case 'm':  state = State::BookmarkRegister; break;
                    case '\'': state = State::BookmarkJump; break;
                    case 's':  state = State::SetMetadataField; break;
                    case 'M':  state = State::ShowMetadataField; break;
                    case 'c':  state = State::Create; break;
                    case 'd':  state = State::Delete; break;
                    case 'D':  state = State::Duplicate; break;
                    case 'p':  state = State::Paste; break;
                    case 'r':  state = State::Rename; break;
                    case 'x':  state = State::Export; break;

                    case 'R': r.commander_reload(); break;
                }
            }
        }

    private:
        Receiver &receiver_() {return *static_cast<Receiver*>(this);}
    };
} } 

#endif
