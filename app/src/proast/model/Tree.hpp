#ifndef HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED

#include <gubg/tree/Node.hpp>
#include <gubg/tree/Forest.hpp>
#include <gubg/markup/Document.hpp>
#include <filesystem>
#include <string>
#include <vector>
#include <ostream>

namespace proast { namespace model { 

    struct Data
    {
        std::filesystem::path path;
        std::string short_name;
        std::size_t active_ix = 0;
        //Replace this with markdown::ast::Tree
        gubg::markup::Document preview;
    };
    using Node = gubg::tree::Node<Data>;
    using Forest = gubg::tree::Forest<Data>;

    using Path = std::vector<std::string>;

    class Tree
    {
    public:
        //Steps down starting from `start` until  the `@root` subfolder is found
        static bool find_root_filepath(std::filesystem::path &root, const std::filesystem::path &start);

        bool load(const std::filesystem::path &root);

        const std::filesystem::path &root_filepath() const;

        void stream(std::ostream &os) const;

        Path root_path() const;

        bool find(const Forest *&forest, std::size_t &ix, const Path &path);

    private:
        std::filesystem::path root_filepath_;
        Forest root_forest_;
        static bool load_(Node &node, const std::string &stem, std::filesystem::path path);
    };

    inline std::ostream &operator<<(std::ostream &os, const Tree &tree)
    {
        tree.stream(os);
        return os;
    }

} } 


#endif
