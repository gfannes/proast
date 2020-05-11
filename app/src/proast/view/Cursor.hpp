#ifndef HEADER_proast_view_Cursor_hpp_ALREADY_INCLUDED
#define HEADER_proast_view_Cursor_hpp_ALREADY_INCLUDED

#include <proast/view/Region.hpp>
#include <proast/log.hpp>
#include <termbox.h>
#include <string>

namespace proast { namespace view { 

    class Cursor
    {
    public:
        Cursor(const Region &region): region_(region) {}

        void write(const std::string &str, bool highlight = false)
        {
            if (row_abs_ >= region_.y_end())
                //Row is out-of-bound
                return;

            tb_cell cell{};
            if (highlight)
                cell.fg = TB_YELLOW;
            for (auto ch: str)
            {
                if (col_abs_ >= region_.x_end())
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
