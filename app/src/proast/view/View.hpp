#ifndef HEADER_proast_view_View_hpp_ALREADY_INCLUDED
#define HEADER_proast_view_View_hpp_ALREADY_INCLUDED

#include <proast/view/Events.hpp>
#include <proast/view/Region.hpp>
#include <proast/view/Cursor.hpp>
#include <proast/presenter/ListBox.hpp>
#include <gubg/mss.hpp>
#include <ncpp.hh>
#include <thread>
#include <vector>

namespace proast { namespace view { 

    class View
    {
    public:
        void set_events_dst(view::Events *events)
        {
            events_ = events;
            if (events_)
                events_->message("Events destination was set");
        }

        void clear_screen()
        {
            auto plane = nc_->get_stdplane();
            if (!!plane)
                plane->erase();
        }

        bool show_mode(const presenter::ListBox &list_box)
        {
            MSS_BEGIN(bool);

            MSS(!!nc_);
            Cursor cursor{mode_region_, *nc_};

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

            MSS(!!nc_);
            Cursor cursor{status_region_, *nc_};

            cursor.write(str);

            MSS_END();
        }

        bool show_parent(const presenter::ListBox &list_box)
        {
            MSS_BEGIN(bool);

            MSS(!!nc_);
            Cursor cursor{parent_region_, *nc_};
            cursor.write("PARENT");

            MSS_END();
        }
        bool show_me(const presenter::ListBox &list_box)
        {
            MSS_BEGIN(bool);

            MSS(!!nc_);
            Cursor cursor{me_region_, *nc_};
            auto show_item = [&](const std::string &item, bool is_active)
            {
                cursor.write(item);
                cursor.newline();
            };
            list_box.each_item(show_item);

            MSS_END();
        }
        bool show_child(const presenter::ListBox &list_box)
        {
            MSS_BEGIN(bool);

            MSS(!!nc_);
            Cursor cursor{child_region_, *nc_};
            auto show_item = [&](const std::string &item, bool is_active)
            {
                cursor.write(item);
                cursor.newline();
            };
            list_box.each_item(show_item);

            MSS_END();
        }

        bool operator()()
        {
            MSS_BEGIN(bool);

            if (!nc_)
                MSS(start_nc_());
            MSS(!!nc_);

            {
                auto region = screen_region_();
                mode_region_ = region.pop_top(1);
                status_region_ = region.pop_bottom(1);

                const auto width = region.width()/7;
                parent_region_ = region.pop_left(width);
                me_region_ = region.pop_left(width);
                child_region_ = region.pop_left(width);
            }

            switch (const auto ch = nc_->getc())
            {
                case 0:
                    //No character present
                    break;
                case -1:
                    //Error
                    break;
                default:
                    events_->message(std::string("Received character: ")+std::to_string(ch));
                    events_->received(ch);
                    break;
            }

            render_screen_();

            MSS_END();
        }

    private:
        bool start_nc_()
        {
            MSS_BEGIN(bool);
            ::notcurses_options nc_options{};
            nc_options.suppress_banner = true;
            nc_.emplace(nc_options, stdout);
            log::stream() << "Created notcurses context" << std::endl;

            log::stream() << screen_region_() << std::endl;

            MSS_END();
        }
        void render_screen_()
        {
            nc_->render();
        }

        Region screen_region_()
        {
            int row_cnt = 0, col_cnt = 0;
            if (!!nc_)
                nc_->get_term_dim(row_cnt, col_cnt);
            return Region(row_cnt, col_cnt);
        }

        view::Events *events_{};

        std::optional<ncpp::NotCurses> nc_;
        Region mode_region_;
        Region parent_region_;
        Region me_region_;
        Region child_region_;
        Region status_region_;
    };

} } 

#endif
