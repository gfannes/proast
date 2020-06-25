#ifndef HEADER_proast_model_Model_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Model_hpp_ALREADY_INCLUDED

#include <proast/model/Events.hpp>
#include <proast/model/Tree.hpp>
#include <proast/model/Config.hpp>
#include <proast/model/Bookmarks.hpp>
#include <proast/model/Markdown.hpp>
#include <proast/log.hpp>
#include <gubg/file/system.hpp>
#include <gubg/naft/Range.hpp>
#include <gubg/OnlyOnce.hpp>
#include <gubg/hr.hpp>
#include <gubg/mss.hpp>
#include <optional>
#include <vector>
#include <list>
#include <chrono>
#include <fstream>
#include <cassert>
#include <filesystem>
#include <cstdlib>

namespace proast { namespace model { 

    enum class Removable
    {
        Node, File, Folder,
    };

    enum class ExportType
    {
        All, Level1, Level2,
    };

    class Model
    {
    public:
        Model(const std::vector<std::filesystem::path> &roots): roots_(roots) {}

        const Config &current_config() const;

        void set_events_dst(model::Events *events);

        //Selecting items
        unsigned int nr_selected() const;
        bool is_selected(const Path &path) const;
        bool select_current();
        bool unselect_all();

        //Reordering items
        bool swap(std::size_t a_ix, std::size_t b_ix);
        bool sort();

        //Setting attributes
        bool set_type(std::optional<Type> type);
        bool set_state(std::optional<State> state);
        bool set_cost(const std::string &cost_str);
        bool set_deadline(const std::string &when_str);

        //Adding items
        //insert==true: nest new item _under_ path_
        //insert==false: add new item _next to_ path_
        bool add_item(const std::string &str, bool insert);
        bool add_selected_links();

        //Reworking items
        bool rename_item(const std::string &new_str);
        bool remove_current(Removable removable);
        bool paste(bool insert);

        //Exporting data
        bool export_nodes(const std::filesystem::path &fp);

        //Bookmarks
        bool register_bookmark(char32_t ch);
        bool load_bookmark(char32_t ch);

        //Processing mainloop
        bool operator()();

        //Location utility
        std::filesystem::path root_filepath(const std::string &name) const;
        std::filesystem::path local_filepath(const Path &path) const;
        std::filesystem::path current_filepath() const;
        std::filesystem::path trash_filepath(const Path &path) const;

        const Path &path() const;
        void set_path(const Path &path);

        bool get(const Forest *&forest, std::size_t &ix, const Path &path) const;
        bool get(const Node *&node, const Path &path) const;
        bool get_parent(Node *&parent, std::size_t &ix, const Path &path);
        bool get_parent(Node *&parent, const Path &path);
        bool get(ConstNodeIXPath &cnixpath, const Path &path);

    private:
        std::filesystem::path user_dir_() const;
        std::filesystem::path metadata_fn_() const;
        std::filesystem::path bookmarks_fn_() const;

        bool update_node_(Node &node, const Node &new_parent) const;

        bool save_content_(const Node &node) const;

        bool save_metadata_() const;
        bool load_metadata_();
        bool reload_();

        const std::vector<std::filesystem::path> roots_;

        model::Events *events_{};
        std::map<std::string, Config> name__config_;
        const Config default_config_;
        std::shared_ptr<Tree> tree_;
        Path path_;

        Bookmarks bookmarks_;

        std::optional<Node> cut_item_;

        std::list<Path> selection_;

        using Clock = std::chrono::high_resolution_clock;
        std::optional<Clock::time_point> save_tp_ = Clock::now();
        Clock::time_point reload_tp_ = Clock::now();
    };

} } 

#endif
