#ifndef HEADER_proast_model_JumpList_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_JumpList_hpp_ALREADY_INCLUDED

#include <proast/model/Node.hpp>
#include <list>

namespace proast { namespace model { 
    class JumpList
    {
    public:
        void append(Node n) { nodes_.push_back(n); }

    private:
        std::list<Node> nodes_;
    };
} } 

#endif
