#ifndef HEADER_proast_view_View_hpp_ALREADY_INCLUDED
#define HEADER_proast_view_View_hpp_ALREADY_INCLUDED

#include <proast/view/Events.hpp>
#include <proast/view/Region.hpp>
#include <proast/view/Cursor.hpp>
#include <proast/presenter/ListBox.hpp>
#include <gubg/mss.hpp>
#include <thread>
#include <vector>

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

        bool show_path(const model::Path &path)
        {
            MSS_BEGIN(bool);

            Cursor cursor{path_region_};

            for (const auto &segment: path)
            {
                cursor.write("/");
                cursor.write(segment);
            }

            MSS_END();
        }
        bool show_mode(const presenter::ListBox &list_box)
        {
            MSS_BEGIN(bool);

            Cursor cursor{mode_region_};

            auto show_item = [&](const std::string &item, bool is_active)
            {
                cursor.write(is_active ? "#" : " ");
                cursor.write(item);
                cursor.write(is_active ? "#" : " ");
            };
            list_box.each_item(show_item);

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
                cursor.write(is_active ? "#" : " ");
                cursor.write(item);
                cursor.write(is_active ? "#" : " ");
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
                cursor.write(is_active ? "#" : " ");
                cursor.write(item);
                cursor.write(is_active ? "#" : " ");
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
                cursor.write(is_active ? "." : " ");
                cursor.write(item);
                cursor.write(is_active ? "." : " ");
                cursor.newline();
            };
            list_box.each_item(show_item);

            MSS_END();
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
                path_region_ = mode_region_ = region.pop_top(1);
                status_region_ = region.pop_bottom(1);

                const auto width = region.width()/7;
                parent_region_ = region.pop_left(width);
                me_region_ = region.pop_left(width);
                child_region_ = region.pop_left(width);
            }

            tb_event event{};
            switch (tb_peek_event(&event, 0))
            {
                case TB_EVENT_KEY:
                    {
                        const auto ch = event.ch;
                        events_->message(std::string("Received character: ")+std::to_string(ch));
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
        Region mode_region_;
        Region parent_region_;
        Region me_region_;
        Region child_region_;
        Region status_region_;
    };

} } 

#endif
