#ifndef HEADER_proast_model_Model_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Model_hpp_ALREADY_INCLUDED

#include <proast/model/Node.hpp>
#include <proast/model/Bookmarks.hpp>
#include <proast/model/JumpList.hpp>
#include <proast/types.hpp>
#include <tuple>
#include <set>
#include <string>

namespace proast { namespace model { 
    class Model
    {
    public:
        Model();
        ~Model();

        bool set_home(const std::filesystem::path &);

        struct Config
        {
            std::set<std::string> names_to_skip;
            std::set<std::string> extensions_to_skip;
            Config();
        };
        bool add_root(const std::filesystem::path &, const Config &);

        bool reload();
        
        bool register_bookmark(char ch);
        bool jump_to_bookmark(char ch);
        template <typename Ftor>
        void each_bookmark(Ftor &&ftor){bookmarks_.each(ftor);}

        bool move(Direction, bool me, bool move_node);

        bool focus(const Path &);
        bool focus(Node);

        bool create(const std::string &name, bool create_file, bool create_in);
        bool rename(const std::string &name);
        bool delete_current();
        bool paste(bool paste_in);

        Node node();
        Node node_0();
        Node node_00();
        Node node_0a();
        Node node_0b();
        Node node_000();
        Node node_00a();
        Node node_00b();
        static std::size_t selected_ix(Node &);

        void recompute_metadata();
        bool save_metadata();
        bool load_metadata();

    private:
        Node root_;
        Node current_node_;
        Node cut_;

        bool add_(Node, const std::filesystem::path &, const Config &);
        void recompute_metadata_(Node);

        std::vector<std::tuple<std::filesystem::path, Config>> root_config_ary_;
        std::filesystem::path home_dir_;
        std::filesystem::path scratchpad_dir_;

        std::filesystem::path bookmarks_fp_;
        Bookmarks bookmarks_;
        JumpList jump_list_;

        std::filesystem::path current_location_fn_;
        bool save_current_location_();
        bool load_current_location_();

        static void compute_navigation_(Node &);

        std::filesystem::path metadata_filename_;
        bool save_metadata_(Node base);
        bool load_metadata_(Node base);

        bool rework_into_directory_(Node);
        bool create_(Node, const std::string &name, bool create_file);

        bool erase_node_(Node);
        bool paste_(Node dst, Node src, const std::filesystem::path &);
        void setup_up_down_(Node &);
    };
} } 

#endif
