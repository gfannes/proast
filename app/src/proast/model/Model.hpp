#ifndef HEADER_proast_model_Model_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Model_hpp_ALREADY_INCLUDED

#include <proast/model/Node.hpp>
#include <proast/model/NodePath.hpp>
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

        bool set_home_dir(const std::filesystem::path &);
        std::filesystem::path get_home_dir() const;

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

        bool move(Movement, bool me, bool move_node);

        bool focus(const StringPath &);
        bool focus(Node);

        bool create(const std::string &name);
        bool rename(const std::string &name);
        bool duplicate(const std::string &name);
        bool append_to_deletes();
        bool clear_deletes();
        bool paste();
        bool do_export(const std::string &name);
        bool search(const std::string &pattern, bool in_content);
        bool plan();
        bool set_node_state(std::optional<State>, bool done);
        bool set_order_sequential(bool order_sequential);

        Node node_a();
        Node node_b();
        Node node_b_pre();
        Node node_b_post();
        Node node_c();
        Node node_c_pre();
        Node node_c_post();

        bool recompute_metadata();
        bool save_metadata();
        bool load_metadata();

        template <typename Ftor>
        void each_delete(Ftor &&ftor)
        {
            for (auto n: deletes_)
                if (n)
                    ftor(n);
        }

    private:
        Node root_;
        NodePath focus_;
        std::list<Node> deletes_;

        bool add_(Node, const std::filesystem::path &, const Config &);
        void recompute_metadata_(Node);

        std::vector<std::tuple<std::filesystem::path, Config>> root_config_ary_;
        std::filesystem::path home_dir_;
        std::filesystem::path scratchpad_dir_;

        std::filesystem::path bookmarks_fp_;
        Bookmarks bookmarks_;
        JumpList jump_list_;

        std::filesystem::path lock_fn_;

        std::filesystem::path current_location_fn_;
        bool save_current_location_();
        bool load_current_location_();

        static void compute_navigation_(Node &);

        std::filesystem::path metadata_filename_;
        bool save_metadata_(Node base);
        bool load_metadata_(Node base);

        bool rework_into_directory_(Node);
        bool create_(Node &, Node, const std::string &name, bool create_file);

        bool erase_node_(Node);
        bool paste_(Node dst, Node src, const std::filesystem::path &);
        void setup_up_down_(Node &);
    };
} } 

#endif
