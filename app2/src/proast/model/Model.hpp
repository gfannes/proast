#ifndef HEADER_proast_model_Model_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Model_hpp_ALREADY_INCLUDED

#include <proast/model/Tree.hpp>
#include <proast/Types.hpp>

namespace proast { namespace model { 
    class Model
    {
    public:
        Tree tree;

        bool add_root(const std::filesystem::path &, const Tree::Config &);

        bool move(Direction);

        const Path &current_path() const {return current_path_;}

        Tree::Node *current_me();
        Tree::Node *current_parent();

    private:
        Path current_path_;
    };
} } 

#endif
