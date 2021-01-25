#ifndef HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED

#include <proast/Data.hpp>
#include <gubg/tree/Node.hpp>
#include <filesystem>
#include <set>

namespace proast { namespace model { 
    using Path = gubg::tree::Path;
    class Tree
    {
    public:
        Tree();

        struct Config
        {
            std::set<std::string> names_to_skip;
            std::set<std::string> extensions_to_skip;
            Config();
        };
        bool add(const std::filesystem::path &, const Config &);

        using Node = gubg::tree::Node<Data>;
        Node root;

        using Datas = std::vector<Data *>;
        bool resolve_datas(Datas &, const Path &);

        using Nodes = std::vector<Node *>;
        bool resolve_nodes(Nodes &, const Path &);

        static std::size_t selected_ix(const Node &);

    private:
        bool add_(Node &, const std::filesystem::path &, const Config &);

    };
} } 

#endif
