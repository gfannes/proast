#ifndef HEADER_proast_model_Node_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Node_hpp_ALREADY_INCLUDED

namespace gubg { namespace tree { 
    template <typename Data> class Node;
} } 

namespace proast { namespace model { 
    class Data;

    using Node = gubg::tree::Node<Data>;
} } 

#endif
