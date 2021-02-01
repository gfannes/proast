#ifndef HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED

#include <proast/model/Data.hpp>
#include <proast/model/Path.hpp>
#include <gubg/tree/Node.hpp>
#include <filesystem>
#include <set>

namespace proast { namespace model { 

    using Node = gubg::tree::Node<Data>;

    Path to_path(Node *node);

    class Tree
    {
    public:
        using Node = model::Node;

        Tree();

        void clear() {*this = Tree();}

        struct Config
        {
            std::set<std::string> names_to_skip;
            std::set<std::string> extensions_to_skip;
            Config();
        };
        bool add(const std::filesystem::path &, const Config &);

        Node root;

        Node *find(const Path &);

        static std::size_t selected_ix(const Node &);

    private:
        bool add_(Node &, const std::filesystem::path &, const Config &);
        static void compute_navigation_(Node &);
    };

} } 

#endif
