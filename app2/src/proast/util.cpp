#include <proast/util.hpp>
#include <fstream>
#include <optional>

namespace proast { 
    std::string to_string(const std::wstring &wstr)
    {
        const auto size = wstr.size();
        std::string str(size, '?');
        for (auto ix = 0u; ix < size; ++ix)
            str[ix] = wstr[ix];
        return str;
    }
    std::wstring to_wstring(const std::string &str)
    {
        const auto size = str.size();
        std::wstring wstr(size, '?');
        for (auto ix = 0u; ix < size; ++ix)
            wstr[ix] = str[ix];
        return wstr;
    }

    std::ostream &log_stream()
    {
        static std::optional<std::ofstream> fo;
        if (!fo)
            fo.emplace("proast.log");
        return *fo;
    }
} 
