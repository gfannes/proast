#ifndef HEADER_proast_model_Node_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Node_hpp_ALREADY_INCLUDED

#include <proast/model/Item.hpp>
#include <gubg/tree/Node.hpp>
#include <gubg/tree/Forest.hpp>

namespace proast { namespace model { 

    using Node = gubg::tree::Node<Item>;
    using Forest = gubg::tree::Forest<Item>;

    bool read_markdown(Node &node, const std::string &markdown);
    bool write_markdown(std::string &markdown, const Node &);

} } 

#endif
