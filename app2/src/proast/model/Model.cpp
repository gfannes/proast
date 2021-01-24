#include <proast/model/Model.hpp>

namespace proast { namespace model { 
    bool Model::add_root(const std::filesystem::path &path, const Tree::Config &config)
    {
        return tree_.add(path, config);
    }
} } 
