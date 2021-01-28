#ifndef HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED

#include <proast/model/Data.hpp>
#include <gubg/tree/Node.hpp>
#include <filesystem>
#include <set>

namespace proast { namespace model { 

    using Node = gubg::tree::Node<Data>;

    using Path = std::vector<int>;

    class Tree
    {
    public:
        using Node = model::Node;

        Tree();

        struct Config
        {
            std::set<std::string> names_to_skip;
            std::set<std::string> extensions_to_skip;
            Config();
        };
        bool add(const std::filesystem::path &, const Config &);

        Node root;

        using Datas = std::vector<Data *>;
        void resolve_datas(Datas &, const Path &);

        using Nodes = std::vector<Node *>;
        void resolve_nodes(Nodes &, const Path &);

        bool is_leaf(const Path &) const;

        static std::size_t selected_ix(const Node &);

    private:
        bool add_(Node &, const std::filesystem::path &, const Config &);
        static void compute_navigation_(Node &);
    };
} } 

#endif
