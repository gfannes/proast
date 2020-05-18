#ifndef HEADER_proast_view_Region_hpp_ALREADY_INCLUDED
#define HEADER_proast_view_Region_hpp_ALREADY_INCLUDED

#include <gubg/macro/capture.hpp>
#include <algorithm>
#include <ostream>

namespace proast { namespace view { 

    class Region
    {
    public:
        Region() {}
        Region(unsigned int y_end, unsigned int x_end): y_end_(y_end), x_end_(x_end) {}
        Region(unsigned int y_begin, unsigned int y_end, unsigned int x_begin, unsigned int x_end): y_begin_(y_begin), y_end_(y_end), x_begin_(x_begin), x_end_(x_end) {}

        void clear() {*this = Region();}

        unsigned int y_begin() const {return y_begin_;}
        unsigned int y_end() const {return y_end_;}
        unsigned int y_size() const {return y_end_-y_begin_;}
        unsigned int height() const {return y_size();}

        unsigned int x_begin() const {return x_begin_;}
        unsigned int x_end() const {return x_end_;}
        unsigned int x_size() const {return x_end_-x_begin_;}
        unsigned int width() const {return x_size();}

        Region pop_left(unsigned int amount)
        {
            Region res = *this;

            amount = std::min(amount, x_end_);
            res.x_end_ -= x_size()-amount;
            x_begin_   += amount;

            return res;
        }
        Region pop_right(unsigned int amount)
        {
            Region res = *this;

            amount = std::min(amount, x_end_);
            res.x_begin_ += x_size()-amount;
            x_end_       -= amount;

            return res;
        }
        Region pop_top(unsigned int amount)
        {
            Region res = *this;

            amount = std::min(amount, y_end_);
            res.y_end_ -= y_size()-amount;
            y_begin_   += amount;

            return res;
        }
        Region pop_bottom(unsigned int amount)
        {
            Region res = *this;

            amount = std::min(amount, y_end_);
            res.y_begin_ += y_size()-amount;
            y_end_       -= amount;

            return res;
        }

        Region pop_all()
        {
            Region res = *this;
            clear();
            return res;
        }

        bool operator==(const Region &rhs) const
        {
#define unroll_fields(ftor) ftor(x_begin_) ftor(y_begin_) ftor(x_end_) ftor(y_end_)
#define ftor(name) if (name != rhs.name) return false;
            unroll_fields(ftor)
#undef ftor
#undef unroll_fields
            return true;
        }

        void stream(std::ostream &os) const
        {
            os << "[Region]" << C(y_begin_)C(y_end_)C(x_begin_)C(x_end_) << std::endl;
        }

    private:
        unsigned int y_begin_ = 0;
        unsigned int y_end_ = 0;

        unsigned int x_begin_ = 0;
        unsigned int x_end_ = 0;
    };

    inline std::ostream &operator<<(std::ostream &os, const Region &region)
    {
        region.stream(os);
        return os;
    }

} } 

#endif
