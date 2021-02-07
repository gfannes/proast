#ifndef HEADER_proast_model_Model_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Model_hpp_ALREADY_INCLUDED

#include <proast/model/Tree.hpp>
#include <proast/model/Bookmarks.hpp>
#include <proast/model/Path.hpp>
#include <proast/model/JumpList.hpp>
#include <proast/types.hpp>
#include <tuple>

namespace proast { namespace model { 
    class Model
    {
    public:
        Model();
        ~Model();

        bool set_home(const std::filesystem::path &);
        bool add_root(const std::filesystem::path &, const Tree::Config &);

        bool reload();
        
        bool register_bookmark(char ch);
        bool jump_to_bookmark(char ch);
        template <typename Ftor>
        void each_bookmark(Ftor &&ftor){bookmarks_.each(ftor);}

        bool move(Direction, bool me);

        bool focus(const Path &);

        bool create_file(const std::string &name, bool in_parent);
        bool create_folder(const std::string &name, bool in_parent);
        bool delete_current();

        Node *node();
        Node *node_0();
        Node *node_00();
        Node *node_0a();
        Node *node_0b();
        Node *node_000();
        Node *node_00a();
        Node *node_00b();

        void recompute_metadata();
        bool sync_metadata();

    private:
        Node *current_node_ = nullptr;

        std::vector<std::tuple<std::filesystem::path, Tree::Config>> root_config_ary_;
        std::filesystem::path home_dir_;

        Tree tree_;

        std::filesystem::path bookmarks_fp_;
        Bookmarks bookmarks_;
        JumpList jump_list_;

        std::filesystem::path current_location_fn_;
        bool save_current_location_();
        bool load_current_location_();
    };
} } 

#endif
