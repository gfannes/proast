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

    class Model
    {
    public:
        Model(const std::vector<std::filesystem::path> &roots): roots_(roots) {}

        const Config &current_config() const
        {
            if (path_.empty())
                return default_config_;
            auto it = name__config_.find(path_[0]);
            if (it == name__config_.end())
                return default_config_;
            return it->second;
        }

        void set_events_dst(model::Events *events)
        {
            events_ = events;
            if (events_)
                events_->message("Events destination was set");
        }

        bool swap(std::size_t a_ix, std::size_t b_ix)
        {
            MSS_BEGIN(bool);

            MSS(!!tree_);

            if (a_ix == b_ix)
                return true;

            Node *parent;
            std::size_t ix;
            MSS(get_parent(parent, ix, path_));

            auto &nodes = parent->childs.nodes;
            MSS(a_ix < nodes.size());
            MSS(b_ix < nodes.size());

            std::swap(nodes[a_ix].value, nodes[b_ix].value);

            MSS(save_content_(*parent));

            MSS_END();
        }
        bool sort()
        {
            MSS_BEGIN(bool);

            Node *parent;
            MSS(get_parent(parent, path_));

            auto to_int = [](std::optional<Type> type)
            {
                int v = 0;
                if (type)
                {
                    if (*type == Type::Requirement) return v; ++v;
                    if (*type == Type::Design) return v; ++v;
                    if (*type == Type::Feature) return v; ++v;
                    if (*type == Type::Free) return v; ++v;
                    if (*type == Type::File) return v; ++v;
                    if (*type == Type::Directory) return v; ++v;
                }
                return v;
            };

            std::stable_sort(RANGE(parent->childs.nodes), [&](const auto &lhs, const auto &rhs){
                    return to_int(lhs.value.type) < to_int(rhs.value.type);
                    });

            MSS(save_content_(*parent));

            MSS_END();
        }
        bool set_type(std::optional<Type> type)
        {
            MSS_BEGIN(bool);
            log::stream() << "Setting " << to_string(path_) << " to " << hr(type) << std::endl;

            Node *node;
            MSS(!!tree_);
            MSS(tree_->find(node, path_));

            if (node->value.type)
            {
                switch (*node->value.type)
                {
                    case Type::File:
                        {
                            //TODO: check that this key does not already exist
                            const std::string key = std::filesystem::path{node->value.key}.stem().string();
                            node->value.key = key;
                            if (!path_.empty())
                            {
                                path_.pop_back();
                                path_.push_back(key);
                            }
                        }
                        break;
                }
            }
            node->value.type = type;

            Node *parent;
            MSS(get_parent(parent, path_));
            MSS(save_content_(*parent));

            MSS_END();
        }
        bool set_state(std::optional<State> state)
        {
            MSS_BEGIN(bool);

            NodeIXPath nixpath;
            MSS(!!tree_);
            MSS(tree_->find(nixpath, path_));

            auto me_node = nixpath.back().node;
            nixpath.pop_back();

            me_node->value.state = state;

            if (!me_node->value.is_embedded())
            {
                MSS(save_content_(*me_node));
            }
            else
            {
                auto parent_node = nixpath.back().node;
                nixpath.pop_back();
                MSS(save_content_(*parent_node));
            }

            MSS_END();
        }
        bool set_cost(const std::string &cost_str)
        {
            MSS_BEGIN(bool);

            MSS(!!tree_);

            Node *node;
            MSS(tree_->find(node, path_));

            try
            {
                if (cost_str.empty())
                    node->value.my_cost.reset();
                else
                    node->value.my_cost = std::stod(cost_str);
            }
            catch (const std::invalid_argument &exc) { return false; }

            MSS(save_content_(*node));
            MSS(reload_());

            MSS_END();
        }

        //insert==true: nest new item _under_ path_
        //insert==false: add new item _next to_ path_
        bool add_item(const std::string &str, bool insert)
        {
            MSS_BEGIN(bool);

            if (!insert)
                if (!path_.empty())
                    path_.pop_back();
            MSS(!path_.empty());

            NodeIXPath nixpath;
            MSS(!!tree_);
            MSS(tree_->find(nixpath, path_));
            assert(nixpath.size() == path_.size());

            auto parent_node = nixpath.back().node;
            //Move index_filename to nonleaf, if needed
            if (insert)
                if (parent_node->value.content_fp && parent_node->value.directory)
                {
                    const auto content_fp_leaf = current_config().content_fp_leaf(*parent_node->value.directory);
                    const auto content_fp_nonleaf = current_config().content_fp_nonleaf(*parent_node->value.directory);
                    if (parent_node->value.content_fp == content_fp_leaf)
                    {
                        std::filesystem::create_directories(*parent_node->value.directory);
                        std::filesystem::rename(content_fp_leaf, content_fp_nonleaf);
                        parent_node->value.content_fp = content_fp_nonleaf;
                    }
                }

            auto &new_item = parent_node->childs.append();
            new_item.value.key = title_as_key(str);
            new_item.value.title = key_as_title(str);
            new_item.value.type = Type::Feature;
            if (parent_node->value.directory)
                new_item.value.directory = *parent_node->value.directory/new_item.value.key;
            if (new_item.value.directory)
            {
                new_item.value.content_fp = current_config().content_fp_leaf(*new_item.value.directory);
                std::ofstream fo{*new_item.value.content_fp};
            }

            MSS(save_content_(new_item));
            MSS(save_content_(*parent_node));

            path_.push_back(new_item.value.key);
            MSS(reload_());

            MSS_END();
        }
        bool add_link(const Path &link_path)
        {
            MSS_BEGIN(bool);

            MSS(!!tree_);

            Node *node;
            MSS(tree_->find(node, path_));

            auto &child = node->childs.append();
            //TODO: Make this consistent with how a link node is added in Tree
            child.value.link = link_path;
            child.value.key = to_string(link_path);

            MSS(save_content_(*node));
            MSS(reload_());

            MSS_END();
        }
        bool rename_item(const std::string &new_str)
        {
            MSS_BEGIN(bool);

            MSS(path_.size() > 1, log::stream() << "Error: Cannot rename the root, you have to do this manually" << std::endl);

            NodeIXPath nixpath;
            MSS(!!tree_);
            MSS(tree_->find(nixpath, path_));

            auto me_node = nixpath.back().node;
            nixpath.pop_back();
            auto parent_node = nixpath.back().node;
            nixpath.pop_back();

            const auto new_key = title_as_key(new_str);
            if (new_key == me_node->value.key)
                //Nothing to rename
                return true;

            me_node->value.key = new_key;

            //Set the title, if not already done so
            bool save_me = false;
            if (me_node->value.title.empty())
            {
                me_node->value.title = key_as_title(new_str);
                save_me = true;
            }

            const auto orig_directory = me_node->value.directory;

            MSS(update_node_(*me_node, *parent_node));

            if (orig_directory && std::filesystem::exists(*orig_directory))
                gubg::file::remove_empty_directories(*orig_directory);

            if (save_me)
                MSS(save_content_(*me_node));
            MSS(save_content_(*parent_node));

            path_.pop_back();
            path_.push_back(new_key);
            MSS(reload_());

            MSS_END();
        }

        bool remove_current(Removable removable)
        {
            MSS_BEGIN(bool);

            MSS(!!tree_);

            NodeIXPath nixpath;
            MSS(tree_->find(nixpath, path_));
            MSS(nixpath.size() >= 2);

            auto me_node = nixpath.back().node;
            auto me_ix = nixpath.back().ix;
            nixpath.pop_back();
            auto parent_node = nixpath.back().node;

            parent_node->childs.remove_if([&](const auto &node){
                    const bool do_remove = &node == me_node;
                    if (do_remove) cut_item_ = node;
                    return do_remove;
                    });
            MSS(save_content_(*parent_node));

            path_.pop_back();
            if (me_ix < parent_node->childs.size())
                path_.push_back(parent_node->childs.nodes[me_ix].value.key);
            else if (parent_node->childs.size() > 1)
                path_.push_back(parent_node->childs.nodes[me_ix-1].value.key);

            if (cut_item_)
                switch (removable)
                {
                    case Removable::Node:
                        break;
                    case Removable::File:
                        if (cut_item_->value.content_fp)
                            std::filesystem::remove(*cut_item_->value.content_fp);
                        cut_item_.reset();
                        break;
                    case Removable::Folder:
                        if (cut_item_->value.content_fp)
                        {
                            if (std::filesystem::is_regular_file(*cut_item_->value.content_fp))
                                std::filesystem::remove(*cut_item_->value.content_fp);
                            else
                                std::filesystem::remove_all(*cut_item_->value.content_fp);
                        }
                        if (cut_item_->value.directory && std::filesystem::exists(*cut_item_->value.directory))
                            std::filesystem::remove_all(*cut_item_->value.directory);
                        cut_item_.reset();
                        break;
                }

            MSS(reload_());

            MSS_END();
        }
        bool paste(bool insert)
        {
            MSS_BEGIN(bool);

            if (!cut_item_)
            {
                log::stream() << "Warning: there is no node to paste" << std::endl;
                return true;
            }

            std::optional<std::size_t> child_ix;

            NodeIXPath nixpath;
            MSS(!!tree_);
            MSS(tree_->find(nixpath, path_));
            assert(nixpath.size() == path_.size());

            if (!insert)
                if (!nixpath.empty())
                {
                    child_ix = nixpath.back().ix;
                    nixpath.pop_back();
                    path_.pop_back();
                }

            auto parent_node = nixpath.back().node;
            if (!child_ix)
                child_ix = parent_node->childs.size();

            auto &child = parent_node->childs.insert(*child_ix);
            child = *cut_item_;

            const auto orig_directory = child.value.directory;

            MSS(update_node_(child, *parent_node));

            if (orig_directory && std::filesystem::exists(*orig_directory))
                gubg::file::remove_empty_directories(*orig_directory);

            cut_item_.reset();

            MSS(save_content_(*parent_node));

            path_.push_back(child.value.key);

            MSS_END();
        }

        bool export_nodes(const std::filesystem::path &fp)
        {
            MSS_BEGIN(bool);

            const Node *node;
            MSS(get(node, path_));

            std::ofstream fo{fp};
            fo << "Key\tMy\tTotal\tDone" << std::endl;
            Path local_path;
            std::vector<bool> p;
            auto writer = [&](const auto &node, const auto &path, unsigned int visit_count)
            {
                if (visit_count == 0 && path.size() == 1)
                {
                    local_path.clear();
                    p.clear(); p.push_back(false);
                }

                double child_cost = 0;
                for (const auto &child: node.childs.nodes)
                    child_cost += child.value.my_cost.value_or(0);

                if (visit_count == 0)
                {
                    const auto &item = node.value;
                    const auto my_cost = item.my_cost.value_or(0);
                    local_path.push_back(item.key);
                    const bool parent_did_export = p.back();
                    const bool do_export = (!parent_did_export && (my_cost > 20 || item.total_cost < 101));
                    if (do_export)
                        fo << to_string(local_path) << "\t" << my_cost << "\t" << item.total_cost << "\t" << item.done_cost << std::endl;
                    p.push_back(do_export || parent_did_export);
                }
                else
                {
                    local_path.pop_back();
                    p.pop_back();
                }
            };
            node->childs.dfs(writer);

            MSS_END();
        }

        bool register_bookmark(char32_t ch)
        {
            MSS_BEGIN(bool);
            bookmarks_.set(ch, path_);
            MSS_END();
        }
        bool load_bookmark(char32_t ch)
        {
            MSS_BEGIN(bool);
            MSS_Q(bookmarks_.get(path_, ch));
            MSS_END();
        }

        bool operator()()
        {
            MSS_BEGIN(bool);

            const auto now = Clock::now();

            if (!tree_ || now >= reload_tp_)
            {
                MSS(reload_());
                reload_tp_ = now+std::chrono::milliseconds(1000);
            }

            if (save_tp_ && now >= *save_tp_)
            {
                //Save from time to time
                if (!save_metadata_())
                {
                    log::stream() << "Warning: Could not save metadata" << std::endl;
                    save_tp_ = std::nullopt;
                }
                else
                    save_tp_ = now+std::chrono::milliseconds(300);
            }

            MSS_END();
        }
        std::filesystem::path root_filepath(const std::string &name) const
        {
            if (!!tree_)
                return tree_->root_filepath(name);
            return std::filesystem::path{};
        }
        std::filesystem::path local_filepath(const Path &path) const
        {
            std::filesystem::path fp;
            if (!path.empty())
            {
                fp = root_filepath(path[0]);
                for (auto ix = 1u; ix < path.size(); ++ix)
                    fp /= path[ix];
            }
            return fp;
        }
        std::filesystem::path current_filepath() const { return local_filepath(path_); }
        std::filesystem::path trash_filepath(const Path &path) const
        {
            auto fp = user_dir_();
            fp /= "trash";
            for (auto ix = 0u; ix < path.size(); ++ix)
                fp /= path[ix];
            return fp;
        }

        const Path &path() const {return path_;}
        void set_path(const Path &path)
        {
            if (path.empty())
            {
                if (!!tree_)
                    path_ = tree_->first_root_path();
                else
                    path_.clear();
                return;
            }

            path_ = path;
        }

        bool get(const Forest *&forest, std::size_t &ix, const Path &path) const
        {
            MSS_BEGIN(bool);

            Forest *my_forest;
            {
                MSS(!!tree_);
                MSS(tree_->find(my_forest, ix, path));
                assert(!!my_forest);
            }
            forest = my_forest;

            MSS_END();
        }
        bool get(const Node *&node, const Path &path) const
        {
            MSS_BEGIN(bool);
            const Forest *forest;
            std::size_t ix;
            MSS(get(forest, ix, path));
            node = &forest->nodes[ix];
            MSS_END();
        }
        bool get_parent(Node *&parent, std::size_t &ix, const Path &path)
        {
            MSS_BEGIN(bool);

            MSS(!path.empty());
            auto parent_path = path;
            parent_path.pop_back();

            Forest *forest;
            {
                MSS(!!tree_);
                MSS_Q(tree_->find(forest, ix, parent_path));
                assert(!!forest);
            }

            parent = &forest->nodes[ix];

            MSS_END();
        }
        bool get_parent(Node *&parent, const Path &path)
        {
            std::size_t ix;
            return get_parent(parent, ix, path);
        }
        bool get(ConstNodeIXPath &cnixpath, const Path &path)
        {
            MSS_BEGIN(bool);
            MSS(!!tree_);
            MSS(tree_->find(cnixpath, path));
            MSS_END();
        }

    private:
        std::filesystem::path user_dir_() const
        {
            std::filesystem::path dir;
            const auto cstr = std::getenv("HOME");
            if (cstr)
                dir = cstr;
            dir /= ".config";
            dir /= "proast";
            return dir;
        }
        std::filesystem::path metadata_fn_() const
        {
            std::filesystem::path fn = user_dir_();
            if (!fn.empty())
                fn /= "metadata.naft";
            return fn;
        }
        std::filesystem::path bookmarks_fn_() const
        {
            std::filesystem::path fn = user_dir_();
            if (!fn.empty())
                fn /= "bookmarks.naft";
            return fn;
        }

        bool update_node_(Node &node, const Node &new_parent) const
        {
            MSS_BEGIN(bool);

            std::vector<const Node *> parent_stack;
            parent_stack.push_back(&new_parent);
            auto my_update_node = [&](auto &node, const auto &path, unsigned int visit_count)
            {
                if (visit_count == 0)
                {
                    if (node.value.content_fp)
                    {
                        assert(!!node.value.directory);
                        const auto orig_content_fp = *node.value.content_fp;
                        const bool is_leaf = (orig_content_fp == current_config().content_fp_leaf(*node.value.directory));
                        node.value.directory = *parent_stack.back()->value.directory/node.value.key;
                        if (is_leaf)
                            node.value.content_fp = current_config().content_fp_leaf(*node.value.directory);
                        else
                            node.value.content_fp = current_config().content_fp_nonleaf(*node.value.directory);

                        std::filesystem::create_directories(node.value.content_fp->parent_path());
                        std::filesystem::rename(orig_content_fp, *node.value.content_fp);
                    }

                    parent_stack.push_back(&node);
                }
                else
                {
                    parent_stack.pop_back();
                }
            };

            gubg::tree::Path p;
            node.dfs(my_update_node, p);

            MSS_END();
        }

        bool save_content_(const Node &node) const
        {
            MSS_BEGIN(bool);
            if (node.value.content_fp)
            {
                std::ofstream fo{*node.value.content_fp};

                std::string markdown;
                MSS(markdown::write_string(markdown, node));

                if (false)
                {
                    log::stream() << "Saving markdown to " << *node.value.content_fp << std::endl;
                    log::stream() << markdown << std::endl;
                }
                MSS(gubg::file::write(markdown, *node.value.content_fp));
            }
            MSS_END();
        }

        bool save_metadata_() const
        {
            MSS_BEGIN(bool);

            MSS(!!tree_);

            const auto metadata_fn = metadata_fn_();
            if (!std::filesystem::exists(metadata_fn))
            {
                const auto user_dir = user_dir_();
                if (!std::filesystem::exists(user_dir))
                    std::filesystem::create_directories(user_dir);
            }

            std::ofstream fo{metadata_fn};
            MSS(fo.good());

            const Node *active_node;
            MSS(get(active_node, path_));

            Path path;
            auto ftor = [&](auto &node, const auto &int_path, auto visit_count)
            {
                MSS_BEGIN(bool);
                if (visit_count == 0)
                {
                    {
                        path.resize(int_path.size());
                        auto it = path.begin();
                        const Forest *forest = &tree_->root_forest();
                        for (auto ix: int_path)
                        {
                            MSS(ix < forest->size());
                            const auto &node = forest->nodes[ix];
                            *it++ = node.value.key;
                            forest = &node.childs;
                        }
                    }

                    fo << '[';
                    gubg::OnlyOnce skip_separator;
                    for (const auto &segment: path)
                    {
                        if (!skip_separator())
                            fo << ':';
                        fo << segment;
                    }
                    fo << ']';
                    if (!node.value.active_child_key.empty())
                        fo << "(active_child_key:" << node.value.active_child_key << ")";
                    if (&node == active_node)
                        fo << "(active)";
                    fo << std::endl;
                }
                MSS_END();
            };
            tree_->root_forest().dfs(ftor);

            MSS(bookmarks_.save(bookmarks_fn_()));

            MSS_END();
        }
        bool load_metadata_()
        {
            MSS_BEGIN(bool);

            MSS(!!tree_);

            std::string content;
            MSS(gubg::file::read(content, metadata_fn_()));

            gubg::naft::Range range{content};

            for (std::string tag; range.pop_tag(tag); )
            {
                Path path;
                {
                    gubg::Strange strange{tag};
                    for (std::string segment; strange.pop_until(segment, ':') || strange.pop_all(segment); )
                        path.push_back(segment);
                }

                gubg::naft::Attrs attrs;
                range.pop_attrs(attrs);

                auto it = attrs.find("active_child_key");
                if (it != attrs.end())
                {
                    Node *node;
                    if (tree_->find(node, path))
                        node->value.active_child_key = it->second;
                }
                it = attrs.find("active");
                if (it != attrs.end())
                    if (path_.empty())
                    {
                        const Node *n;
                        if (get(n, path))
                            path_ = path;
                    }
            }

            if (std::filesystem::exists(bookmarks_fn_()))
                MSS(bookmarks_.load(bookmarks_fn_()));

            MSS_END();
        }
        bool reload_()
        {
            MSS_BEGIN(bool);

            std::shared_ptr<Tree> tree{new Tree};
            name__config_.clear();

            for (const auto &root: roots_)
            {
                L(C(root));
                Config::create_default(root);

                const std::string name = root.stem().string();
                auto &config = name__config_[name];

                const auto config_fp = Config::filepath(root);
                MSS(config.reload(config_fp), log::stream() << "Error: Could not load the configuration from " << config_fp << std::endl);

                MSS(tree->load(root, config));
            }

            //Compute aggregates
            MSS(tree->compute_aggregates());

            tree_.swap(tree);

            if (!load_metadata_())
                log::stream() << "Warning: Could not load metadata" << std::endl;

            if (path_.empty())
            {
                MSS(!!tree_);
                path_ = tree_->first_root_path();
            }

            MSS_END();
        }

        const std::vector<std::filesystem::path> roots_;

        model::Events *events_{};
        std::map<std::string, Config> name__config_;
        const Config default_config_;
        std::shared_ptr<Tree> tree_;
        Path path_;

        Bookmarks bookmarks_;

        std::optional<Node> cut_item_;

        using Clock = std::chrono::high_resolution_clock;
        std::optional<Clock::time_point> save_tp_ = Clock::now();
        Clock::time_point reload_tp_ = Clock::now();
    };

} } 

#endif
