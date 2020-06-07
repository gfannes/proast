#ifndef HEADER_proast_model_Node_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Node_hpp_ALREADY_INCLUDED

#include <proast/model/Item.hpp>
#include <gubg/tree/Node.hpp>
#include <gubg/tree/Forest.hpp>
#include <vector>

namespace proast { namespace model { 

    using Node = gubg::tree::Node<Item>;
    using Forest = gubg::tree::Forest<Item>;

    struct NodeIX
    {
        Node *node = nullptr;
        std::size_t ix = 0;

        NodeIX() {}
        NodeIX(Node *node): node(node) {}
        NodeIX(Node *node, std::size_t ix): node(node), ix(ix) {}
    };
    using NodeIXPath = std::vector<NodeIX>;

} } 

#endif
