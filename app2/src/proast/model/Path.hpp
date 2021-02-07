#ifndef HEADER_proast_model_Path_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Path_hpp_ALREADY_INCLUDED

#include <proast/model/Node.hpp>
#include <vector>
#include <string>

namespace proast { namespace model { 

    using Path = std::vector<std::string>;

    std::string to_string(const Path &);
    Path to_path(const std::string &);
    Path to_path(Node *);

    bool pop_if(Path &path, const Path &needle);

} } 

#endif
