#include <proast/model/StringPath.hpp>
#include <sstream>
#include <string_view>

namespace proast { namespace model { 

    std::string to_string(const StringPath &path)
    {
        std::ostringstream oss;
        for (const auto &e: path)
            oss << '/' << e;
        return oss.str();
    }
    StringPath to_string_path(const std::string &str)
    {
        const auto sep = '/';

        StringPath path;

        std::string_view sv{str};

        auto pop_sep = [&](){
            if (sv.starts_with(sep))
                sv.remove_prefix(1);
        };

        for (std::string part; (pop_sep(), !sv.empty()); path.push_back(part))
        {
            auto pop_part = [&](std::size_t count){
                part = sv.substr(0, count);
                sv.remove_prefix(count);
            };

            if (const auto ix = sv.find(sep); ix == sv.npos)
                pop_part(sv.size());
            else
                pop_part(ix);
        }

        return path;
    }

    bool pop_if(StringPath &path, const StringPath &needle)
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
