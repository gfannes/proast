#ifndef HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED

#include <proast/model/Config.hpp>
#include <proast/model/Path.hpp>
#include <gubg/tree/Node.hpp>
#include <gubg/tree/Forest.hpp>
#include <gubg/markup/Document.hpp>
#include <filesystem>
#include <string>
#include <ostream>
#include <optional>
#include <map>

namespace proast { namespace model { 

    struct Data
    {
        std::optional<std::filesystem::path> directory;
        std::optional<std::filesystem::path> content_fp;

        std::optional<Path> link;

        std::string short_name;
        std::string active_child_key;
        //Replace this with markdown::ast::Tree
        gubg::markup::Document preview;

        std::optional<double> my_cost;
    };
    using Node = gubg::tree::Node<Data>;
    using Forest = gubg::tree::Forest<Data>;

    class Tree
    {
    public:
        //Steps down starting from `start` until  the `@root` subfolder is found
        static bool find_root_filepath(std::filesystem::path &root, const std::filesystem::path &start);

        bool load(const std::filesystem::path &root, const Config &cfg);

        std::filesystem::path root_filepath(const std::string &name) const;

        void stream(std::ostream &os) const;

        Path first_root_path() const;

        bool find(Forest *&forest, std::size_t &ix, const Path &path);
        bool find(Node *&node, const Path &path);

        Forest &root_forest() {return root_forest_;}
        const Forest &root_forest() const {return root_forest_;}

    private:
        bool load_(const Config &cfg, Node &node, const std::string &stem, std::filesystem::path path) const;

        std::map<std::string, Config> name__cfg_;
        std::map<std::string, std::filesystem::path> name__root_filepath_;
        Forest root_forest_;
    };

    inline std::ostream &operator<<(std::ostream &os, const Tree &tree)
    {
        tree.stream(os);
        return os;
    }

} } 


#endif
