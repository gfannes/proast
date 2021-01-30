#ifndef HEADER_proast_model_Model_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Model_hpp_ALREADY_INCLUDED

#include <proast/model/Tree.hpp>
#include <proast/model/Bookmarks.hpp>
#include <proast/model/Path.hpp>
#include <proast/types.hpp>

namespace proast { namespace model { 
    class Model
    {
    public:
        Model();
        ~Model();

        bool set_home(const std::filesystem::path &);
        bool add_root(const std::filesystem::path &, const Tree::Config &);
        
        bool register_bookmark(wchar_t wchar);
        bool jump_to_bookmark(wchar_t wchar);

        bool move(Direction, bool me);

        Node *node();
        Node *node_0();
        Node *node_00();
        Node *node_0a();
        Node *node_0b();
        Node *node_000();
        Node *node_00a();
        Node *node_00b();

        Path to_path(Node *node) const;

    private:
        Node *current_node_ = nullptr;

        Tree tree_;

        std::filesystem::path bookmarks_fp_;
        Bookmarks bookmarks_;
    };
} } 

#endif
