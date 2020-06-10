#ifndef HEADER_proast_view_Cursor_hpp_ALREADY_INCLUDED
#define HEADER_proast_view_Cursor_hpp_ALREADY_INCLUDED

#include <proast/view/Region.hpp>
#include <proast/log.hpp>
#include <gubg/markup/Style.hpp>
#include <termbox.h>
#include <string>

namespace proast { namespace view { 

    class Cursor
    {
    public:
        Cursor(const Region &region, int margin = 0): region_(region), margin_(margin) {}

        void fill(char ch)
        {
            tb_cell cell{};
            cell.ch = ch;
            for (auto x = region_.x_begin(); x < region_.x_end(); ++x)
                for (auto y = region_.y_begin(); y < region_.y_end(); ++y)
                    tb_put_cell(x, y, &cell);
        }

        void write(const std::string &str, const gubg::markup::Style style = gubg::markup::Style{})
        {
            if (row_abs_ >= region_.y_end()-margin_)
                //Row is out-of-bound
                return;

            tb_cell cell{};
            switch (style.attention)
            {
                case 0: cell.fg = TB_WHITE; break;
                case 1: cell.fg = TB_BLUE; break;
                case 2: cell.fg = TB_YELLOW; break;
                case 3: cell.fg = TB_GREEN; break;
                case 4: cell.fg = TB_MAGENTA; break;
                case 5: cell.fg = TB_CYAN; break;
                case 6: cell.fg = TB_RED; break;
                default: cell.fg = TB_RED; break;
            }
            if (style.bold)
                cell.fg |= TB_BOLD;
            if (style.italic)
                cell.fg |= TB_REVERSE;
            if (style.done)
                cell.bg = TB_GREEN;
            for (auto ch: str)
            {
                if (col_abs_ >= region_.x_end()-margin_)
                    //Col is out-of-bound
                    return;
                cell.ch = ch;
                tb_put_cell(col_abs_, row_abs_, &cell);
                ++col_abs_;
            }
        }

        void goto_row(unsigned int row)
        {
            row_abs_ = region_.y_begin()+row;
            col_abs_ = region_.x_begin();
        }

        void newline()
        {
            if (row_abs_ < region_.y_end())
                ++row_abs_;
            col_abs_ = region_.x_begin();
        }

    private:
        const Region &region_;
        int margin_;
        unsigned int row_abs_ = region_.y_begin();
        unsigned int col_abs_ = region_.x_begin();
    };

} } 

#endif
