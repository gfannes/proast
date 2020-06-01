#ifndef HEADER_proast_view_View_hpp_ALREADY_INCLUDED
#define HEADER_proast_view_View_hpp_ALREADY_INCLUDED

#include <proast/view/Events.hpp>
#include <proast/view/Region.hpp>
#include <proast/view/Cursor.hpp>
#include <proast/presenter/ListBox.hpp>
#include <proast/presenter/Dialog.hpp>
#include <gubg/mss.hpp>
#include <thread>
#include <vector>
#include <map>

namespace proast { namespace view { 

    class View
    {
    public:
        ~View()
        {
            if (termbox_ok_)
            {
                std::cout << "Shutdown" << std::endl;
                tb_shutdown();
            }
        }

        void set_events_dst(view::Events *events)
        {
            events_ = events;
            if (events_)
                events_->message("Events destination was set");
        }

        void clear_screen()
        {
            if (!termbox_ok_)
                return;
            tb_clear();
        }
        void render_screen()
        {
            tb_present();
        }

        bool show_location(const std::string &str)
        {
            MSS_BEGIN(bool);

            Cursor cursor{path_region_};
            cursor.write(str);

            MSS_END();
        }

        bool show_status(const std::string &str)
        {
            MSS_BEGIN(bool);

            Cursor cursor{status_region_};

            cursor.write(str);

            MSS_END();
        }

        bool show_parent(const presenter::ListBox &list_box)
        {
            MSS_BEGIN(bool);

            Cursor cursor{parent_region_};

            auto show_item = [&](const std::string &item, bool is_active)
            {
                cursor.write(item, is_active);
                cursor.newline();
            };
            list_box.each_item(show_item);

            MSS_END();
        }
        bool show_me(const presenter::ListBox &list_box)
        {
            MSS_BEGIN(bool);

            Cursor cursor{me_region_};

            auto show_item = [&](const std::string &item, bool is_active)
            {
                cursor.write(item, is_active);
                cursor.newline();
            };
            list_box.each_item(show_item);

            MSS_END();
        }
        bool show_child(const presenter::ListBox &list_box)
        {
            MSS_BEGIN(bool);

            Cursor cursor{child_region_};

            auto show_item = [&](const std::string &item, bool is_active)
            {
                cursor.write(item, is_active);
                cursor.newline();
            };
            list_box.each_item(show_item);

            MSS_END();
        }
        bool show_preview(const gubg::markup::Document &doc)
        {
            MSS_BEGIN(bool);

            Cursor cursor{preview_region_};

            std::size_t prev_line_ix = 0;
            auto show = [&](std::size_t line_ix, const std::string &txt, const gubg::markup::Style &style)
            {
                for (; prev_line_ix < line_ix; ++prev_line_ix)
                    cursor.newline();
                cursor.write(txt, style.attention > 0);
            };
            doc.each(show);

            MSS_END();
        }
        bool show_details(const std::map<std::string, std::string> &kv)
        {
            MSS_BEGIN(bool);

            Cursor cursor{details_region_};

            for (const auto &[k,v]: kv)
            {
                cursor.write(k, true);
                cursor.write(": ", false);
                cursor.write(v, false);
                cursor.newline();
            }

            MSS_END();
        }
        bool show_dialog(const presenter::Dialog &dialog)
        {
            MSS_BEGIN(bool);

            Cursor cursor{dialog_region_};
            cursor.fill('.');
            cursor.write(dialog.caption(), true);
            cursor.newline();
            cursor.newline();
            cursor.write(dialog.content(), false);

            MSS_END();
        }

        template <typename Ftor>
        void pause(Ftor &&ftor)
        {
            if (termbox_ok_)
                tb_shutdown();

            ftor();

            if (termbox_ok_)
                tb_init();
        }

        bool operator()()
        {
            MSS_BEGIN(bool);

            if (!termbox_ok_)
            {
                MSS(tb_init() == 0);
                termbox_ok_ = true;
            }

            {
                auto region = screen_region_();
                path_region_ = region.pop_top(1);
                status_region_ = region.pop_bottom(1);

                const auto width = region.width()/7;
                {
                    const auto height = region.height()/7;
                    dialog_region_ = region;
                    dialog_region_.pop_border(height, width);
                }
                parent_region_ = region.pop_left(width);
                me_region_ = region.pop_left(width);
                child_region_ = region.pop_left(width);

                {
                    const auto height = region.height()/5;
                    details_region_ = region.pop_bottom(height);
                    preview_region_ = region.pop_all();
                }
            }

            tb_event event{};
            switch (tb_peek_event(&event, 0))
            {
                case TB_EVENT_KEY:
                    {
                        if (false)
                            events_->message(std::string("Received event.ch:")+std::to_string(event.ch)+", event.key:"+std::to_string(event.key));

                        uint32_t ch;
                        switch (event.key)
                        {
                            case TB_KEY_ENTER:        ch = '\n'; break;
                            case TB_KEY_ESC:          ch = 0x1B; break;
                            case 0x7F:                ch = 0x7F; break;//Backspace
                            case TB_KEY_ARROW_UP:     ch = 0x80; break;
                            case TB_KEY_ARROW_DOWN:   ch = 0x81; break;
                            case TB_KEY_ARROW_LEFT:   ch = 0x82; break;
                            case TB_KEY_ARROW_RIGHT:  ch = 0x83; break;
                            default:
                                ch = event.ch;
                                break;
                        }

                        if (false)
                            events_->message(std::string("Received character: ")+std::to_string((int)ch));

                        events_->received(ch);
                    }
                    break;
            }

            MSS_END();
        }

    private:
        Region screen_region_()
        {
            int row_cnt = 0, col_cnt = 0;
            if (termbox_ok_)
            {
                row_cnt = tb_height();
                col_cnt = tb_width();
            }
            return Region(row_cnt, col_cnt);
        }

        view::Events *events_{};

        bool termbox_ok_ = false;
        Region path_region_;
        Region parent_region_;
        Region me_region_;
        Region child_region_;
        Region status_region_;
        Region preview_region_;
        Region details_region_;
        Region dialog_region_;
    };

} } 

#endif
