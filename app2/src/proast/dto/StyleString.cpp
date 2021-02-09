#include <proast/dto/StyleString.hpp>

namespace proast { namespace dto { 
    int StyleString::attention(unsigned int ix) const
    {
        auto it = ix__attention.upper_bound(ix);
        if (it == ix__attention.begin())
            return 0;
        --it;
        return it->second;
    }
    bool StyleString::bold(unsigned int ix) const
    {
        auto it = ix__bold.upper_bound(ix);
        if (it == ix__bold.begin())
            return 0;
        --it;
        return it->second;
    }
} } 
