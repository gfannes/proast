#ifndef HEADER_proast_util_hpp_ALREADY_INCLUDED
#define HEADER_proast_util_hpp_ALREADY_INCLUDED

#include <string>
#include <ostream>

namespace proast { 
    std::string to_string(const std::wstring &wstr);
    std::wstring to_wstring(const std::string &str);

    std::ostream &log_stream();

    template <typename Ftor>
    void log(Ftor &&ftor)
    {
        auto &os = log_stream();
        ftor(os);
        std::flush(os);
    }
} 

#endif
