#ifndef HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED

#include <gubg/tree/Node.hpp>
#include <gubg/tree/Forest.hpp>
#include <filesystem>
#include <string>
#include <vector>
#include <ostream>

namespace proast { namespace model { 

    struct Data
    {
        std::string short_name;
        std::size_t active_ix = 0;
    };
    using Node = gubg::tree::Node<Data>;
    using Forest = gubg::tree::Forest<Data>;

    using Path = std::vector<std::string>;

    class Tree
    {
    public:
        //Steps down starting from `start` until  the `@root` subfolder is found
        static bool find_root(std::filesystem::path &root, const std::filesystem::path &start);

        bool load(const std::filesystem::path &root);

        const std::filesystem::path &root_path() const;

        void stream(std::ostream &os) const;

        bool find(const Forest *&forest, std::size_t &ix, const Path &path);

    private:
        std::filesystem::path root_path_;
        Forest root_forest_;
        static bool load_(Node &node, std::filesystem::path path);
    };

    inline std::ostream &operator<<(std::ostream &os, const Tree &tree)
    {
        tree.stream(os);
        return os;
    }

} } 


#endif
