#include <proast/model/NodePath.hpp>

namespace proast { namespace model { 
    StringPath to_string_path(const NodePath &node_path)
    {
        StringPath string_path;
        for (auto ix = 1u; ix < node_path.size(); ++ix)
        {
            auto &n = node_path[ix];
            if (n)
                string_path.push_back(n->name());
        }
        return string_path;
    }
} } 
