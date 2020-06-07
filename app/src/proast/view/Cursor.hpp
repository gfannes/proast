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
        Cursor(const Region &region): region_(region) {}

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
            const int margin = 1;
            if (row_abs_ >= region_.y_end()-margin)
                //Row is out-of-bound
                return;

            tb_cell cell{};
            switch (style.attention)
            {
                case 0: cell.fg = TB_BLUE; break;
                case 1: cell.fg = TB_YELLOW; break;
            }
            for (auto ch: str)
            {
                if (col_abs_ >= region_.x_end()-margin)
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
        unsigned int row_abs_ = region_.y_begin();
        unsigned int col_abs_ = region_.x_begin();
    };

} } 

#endif
