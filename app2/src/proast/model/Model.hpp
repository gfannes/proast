#ifndef HEADER_proast_model_Model_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Model_hpp_ALREADY_INCLUDED

#include <proast/model/Tree.hpp>
#include <proast/model/Bookmarks.hpp>
#include <proast/model/Path.hpp>
#include <proast/model/JumpList.hpp>
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
        template <typename Ftor>
        void each_bookmark(Ftor &&ftor){bookmarks_.each(ftor);}

        bool move(Direction, bool me);

        Node *node();
        Node *node_0();
        Node *node_00();
        Node *node_0a();
        Node *node_0b();
        Node *node_000();
        Node *node_00a();
        Node *node_00b();

    private:
        Node *current_node_ = nullptr;

        Tree tree_;

        std::filesystem::path bookmarks_fp_;
        Bookmarks bookmarks_;
        JumpList jump_list_;
    };
} } 

#endif
