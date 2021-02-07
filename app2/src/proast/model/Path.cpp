#include <proast/model/Path.hpp>
#include <proast/model/Tree.hpp>
#include <proast/util.hpp>
#include <sstream>
#include <string_view>

namespace proast { namespace model { 

    std::string to_string(const Path &path)
    {
        std::ostringstream oss;
        for (const auto &e: path)
            oss << '/' << e;
        return oss.str();
    }
    Path to_path(const std::string &str)
    {
        const auto sep = '/';

        Path path;

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
    Path to_path(Node *node)
    {
        Path p;
        while (node)
        {
            auto parent = node->value.navigation.parent;
            if (parent)
                //We do not include the root node name
                p.push_back(node->value.name);
            node = parent;
        }
        std::reverse(p.begin(), p.end());
        return p;
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
