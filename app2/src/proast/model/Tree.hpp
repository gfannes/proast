#ifndef HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED

#include <proast/Data.hpp>
#include <gubg/tree/Forest.hpp>
#include <filesystem>
#include <set>

namespace proast { namespace model { 
    using Path = gubg::tree::Path;
    class Tree
    {
    public:
        struct Config
        {
            std::set<std::string> names_to_skip;
            std::set<std::string> extensions_to_skip;
            Config();
        };
        bool add(const std::filesystem::path &, const Config &);

    private:
        using Forest = gubg::tree::Forest<Data>;

        bool add_(Forest &, const std::filesystem::path &, const Config &);

        Forest forest_;
    };
} } 

#endif
