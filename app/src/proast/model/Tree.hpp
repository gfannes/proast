#ifndef HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED

#include <proast/model/Path.hpp>
#include <gubg/tree/Node.hpp>
#include <filesystem>
#include <string>
#include <ostream>

namespace proast { namespace model { 

    struct Data
    {
        std::string short_name;
    };
    using Node = gubg::tree::Node<Data>;

    class Tree
    {
    public:
        //Steps down starting from `start` until  the `@root` subfolder is found
        static bool find_root(std::filesystem::path &root, const std::filesystem::path &start);

        bool load(const std::filesystem::path &root);

        const std::filesystem::path &root_path() const;

        void stream(std::ostream &os) const;

        const Node *find(const Path &path);

    private:
        std::filesystem::path root_path_;
        Node root_;
        static bool load_(Node &node, std::filesystem::path path);
    };

    inline std::ostream &operator<<(std::ostream &os, const Tree &tree)
    {
        tree.stream(os);
        return os;
    }

} } 


#endif
