#ifndef HEADER_proast_model_Model_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Model_hpp_ALREADY_INCLUDED

#include <proast/model/Tree.hpp>

namespace proast { namespace model { 
    class Model
    {
    public:
        bool add_root(const std::filesystem::path &, const Tree::Config &);

    private:
        Tree tree_;
    };
} } 

#endif
