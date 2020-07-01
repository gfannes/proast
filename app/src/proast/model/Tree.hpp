#ifndef HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Tree_hpp_ALREADY_INCLUDED

#include <proast/model/Config.hpp>
#include <proast/model/Path.hpp>
#include <proast/model/Node.hpp>
#include <gubg/markup/Document.hpp>
#include <filesystem>
#include <string>
#include <ostream>
#include <optional>
#include <map>

namespace proast { namespace model { 

    class Tree
    {
    public:
        static bool find_root_filepath(std::filesystem::path &root, const std::filesystem::path &start);

        bool load(const std::filesystem::path &root, const Config &cfg);

        std::filesystem::path root_filepath(const std::string &name) const;

        void stream(std::ostream &os) const;

        Path first_root_path() const;

        bool find(Forest *&forest, std::size_t &ix, const Path &path);
        bool find(Node *&node, const Path &path);
        bool find(NodeIXPath &nixpath, const Path &path);
        bool find(ConstNodeIXPath &cnixpath, const Path &path) const;

        Forest &root_forest() {return root_forest_;}
        const Forest &root_forest() const {return root_forest_;}

        bool compute_aggregates();
        bool set_paths();

        template <typename Ftor>
        void each_node(Ftor &&ftor)
        {
            auto my_ftor = [&](Node &n, const auto &path, unsigned int visit_count)
            {
                if (visit_count == 0)
                    ftor(n);
            };
            root_forest_.dfs(my_ftor);
        }

    private:
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
