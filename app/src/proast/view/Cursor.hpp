#ifndef HEADER_proast_view_Cursor_hpp_ALREADY_INCLUDED
#define HEADER_proast_view_Cursor_hpp_ALREADY_INCLUDED

#include <proast/view/Region.hpp>
#include <ncpp.hh>
#include <string>

namespace proast { namespace view { 

    class Cursor
    {
    public:
        Cursor(const Region &region, ncpp::NotCurses &nc): region_(region), plane_(nc.get_stdplane()) {}

        void write(const std::string &str)
        {
            if (!plane_)
                return;
            if (row_abs_ >= region_.y_end())
                //Row is out-of-bound
                return;

            for (auto ch: str)
            {
                if (col_abs_ >= region_.x_end())
                    //Col is out-of-bound
                    return;
                plane_->putc(row_abs_, col_abs_, ch);
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
        ncpp::Plane *plane_;
        unsigned int row_abs_ = region_.y_begin();
        unsigned int col_abs_ = region_.x_begin();
    };

} } 

#endif
