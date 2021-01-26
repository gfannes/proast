#ifndef HEADER_proast_util_hpp_ALREADY_INCLUDED
#define HEADER_proast_util_hpp_ALREADY_INCLUDED

#include <string>

namespace proast { 
    inline std::string to_string(const std::wstring &wstr)
    {
        const auto size = wstr.size();
        std::string str(size, '?');
        for (auto ix = 0u; ix < size; ++ix)
            str[ix] = wstr[ix];
        return str;
    }
    inline std::wstring to_wstring(const std::string &str)
    {
        const auto size = str.size();
        std::wstring wstr(size, '?');
        for (auto ix = 0u; ix < size; ++ix)
            wstr[ix] = str[ix];
        return wstr;
    }
} 

#endif
