#include <proast/util.hpp>
#include <locale>
#include <codecvt>

namespace proast { 
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> s_converter;

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
        auto wstr = s_converter.from_bytes(str);
        //Convert non-printable characters to '~'; ftxui will hang on these
        for (auto &wch: wstr)
            if (wch < 32)
                wch = '~';
        return wstr;
    }
    std::string to_utf8(const std::wstring &wstr)
    {
        auto str = s_converter.to_bytes(wstr);
        return str;
    }
} 
