#ifndef HEADER_proast_view_View_hpp_ALREADY_INCLUDED
#define HEADER_proast_view_View_hpp_ALREADY_INCLUDED

#include <proast/view/Events.hpp>
#include <proast/model/ListBox.hpp>
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

        bool show_mode(const model::ListBox &list_box)
        {
            MSS_BEGIN(bool);

            MSS(!!nc_);
            auto plane = nc_->get_stdplane();
            MSS(!!plane);

            int row = 0, col = 0;
            auto set = [&](const std::string &str)
            {
                for (auto ch: str)
                {
                    if (col >= col_cnt_)
                        return;
                    plane->putc(row, col, ch);
                    ++col;
                }
            };
            auto show_item = [&](const std::string &item, bool is_active)
            {
                set(is_active ? "#" : " ");
                set(item);
                set(is_active ? "#" : " ");
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

            if (false && ch_ <= 'z')
            {
                fill_screen_(ch_);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                ++ch_;
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

            nc_->render();

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
            {
                nc_->get_term_dim(row_cnt_,col_cnt_);
                log::stream() << "Row count: " << row_cnt_ << ", Col count: " << col_cnt_ << "" << std::endl;
            }
            MSS_END();
        }

        bool fill_screen_(const char ch)
        {
            MSS_BEGIN(bool);
            auto plane = nc_->get_stdplane();
            MSS(!!plane);

            for (auto rix = 0u; rix < row_cnt_; ++rix)
            {
                for (auto cix = 0u; cix < col_cnt_; ++cix)
                {
                    plane->putc(rix, cix, ch);
                }
            }

            MSS_END();
        }

        view::Events *events_{};

        std::optional<ncpp::NotCurses> nc_;
        int row_cnt_{}, col_cnt_{};

        char ch_ = 'a';
    };

} } 

#endif
