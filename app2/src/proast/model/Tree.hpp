#ifndef HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED

#include <proast/model/Data.hpp>
#include <proast/model/Path.hpp>
#include <gubg/tree/Node.hpp>
#include <filesystem>
#include <set>
#include <map>

namespace proast { namespace model { 

    using Node = gubg::tree::Node<Data>;

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

        static void recompute_metadata(Node &);
        bool stream_metadata();

        static std::size_t selected_ix(const Node &);

    private:
        bool add_(Node &, const std::filesystem::path &, const Config &);
        static void compute_navigation_(Node &);

        static bool stream_metadata_(std::ostream &, Node &);

        using Path__Metadata = std::map<Path, Metadata>;
        Path__Metadata path__metadata_;
        bool append_metadata_(const std::filesystem::path &);
        void set_metadata_();
        static std::filesystem::path metadata_fn_(const std::filesystem::path &base_dir);
    };

} } 

#endif
