#ifndef HEADER_proast_util_hpp_ALREADY_INCLUDED
#define HEADER_proast_util_hpp_ALREADY_INCLUDED

#include <string>

namespace proast { 
    std::string to_string(const std::wstring &wstr);
    std::wstring to_wstring(const std::string &str);
} 

#endif
