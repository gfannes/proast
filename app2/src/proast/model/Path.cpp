#include <proast/model/Path.hpp>
#include <sstream>
#include <string_view>

namespace proast { namespace model { 

    std::wstring to_wstring(const Path &path)
    {
        std::basic_ostringstream<wchar_t> oss;
        for (const auto &e: path)
            oss << L'/' << e;
        return oss.str();
    }
    Path to_path(const std::wstring &wstr)
    {
        const auto sep = L'/';

        Path path;

        std::wstring_view wstrv{wstr};

        auto pop_sep = [&](){
            if (wstrv.starts_with(sep))
                wstrv.remove_prefix(1);
        };

        for (std::wstring part; !wstrv.empty(); path.push_back(part), pop_sep())
        {
            auto pop_part = [&](std::size_t count){
                part = wstrv.substr(count);
                wstrv.remove_prefix(count);
            };

            if (const auto ix = wstrv.find(sep); ix == wstrv.npos)
                pop_part(wstrv.size());
            else
                pop_part(ix);
        }

        return path;
    }

    bool pop_if(Path &path, const Path &needle)
    {
        const auto needle_size = needle.size();

        if (needle_size > path.size())
            return false;

        for (auto ix = 0u; ix < needle_size; ++ix)
            if (needle[ix] != path[ix])
                return false;

        path.erase(path.begin(), path.begin()+needle_size);

        return true;
    }

} } 
