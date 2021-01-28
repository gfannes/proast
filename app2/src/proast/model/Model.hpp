#ifndef HEADER_proast_model_Model_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Model_hpp_ALREADY_INCLUDED

#include <proast/model/Tree.hpp>
#include <proast/Types.hpp>

namespace proast { namespace model { 
    class Model
    {
    public:
        Tree tree;

        Model();

        bool add_root(const std::filesystem::path &, const Tree::Config &);

        bool move(Direction, bool me);

        Node *node();
        Node *node_0();
        Node *node_00();
        Node *node_0a();
        Node *node_0b();
        Node *node_000();
        Node *node_00a();
        Node *node_00b();

    private:
        Node *node_ = nullptr;
    };
} } 

#endif
