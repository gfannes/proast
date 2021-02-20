#ifndef HEADER_proast_model_NodePath_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_NodePath_hpp_ALREADY_INCLUDED

#include <proast/model/Node.hpp>
#include <vector>

namespace proast { namespace model { 

    using NodePath = std::vector<Node>;

    StringPath to_string_path(const NodePath &);

} } 

#endif
