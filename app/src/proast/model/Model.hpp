#ifndef HEADER_proast_model_Model_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Model_hpp_ALREADY_INCLUDED

#include <proast/model/Events.hpp>
#include <proast/model/Tree.hpp>
#include <proast/model/Config.hpp>
#include <proast/model/Bookmarks.hpp>
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

        bool add_item(const std::string &short_name, bool insert)
        {
            MSS_BEGIN(bool);

            if (!insert)
            {
                if (path_.size() > 1)
                    path_.pop_back();
            }

            path_.push_back(short_name);
            auto fp = local_filepath(path_);
            log::stream() << "Note: Creating folder " << fp << std::endl;
            std::filesystem::create_directories(fp);
            fp /= "readme.md";
            std::ofstream fo{fp};

            MSS(fo.good());
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
            child.value.short_name = to_string(link_path);

            MSS(save_content_(*node));
            MSS(reload_());

            MSS_END();
        }
        bool rename_item(const std::string &new_short_name)
        {
            MSS_BEGIN(bool);

            MSS(path_.size() > 1, log::stream() << "Error: Cannot rename the root, you have to do this manually" << std::endl);

            const Node *me;
            MSS(get(me, path_));

            auto new_path = path_;
            {
                MSS(!new_path.empty());
                new_path.pop_back();
                new_path.push_back(new_short_name);
            }

            MSS(!!me->value.directory);
            const auto &directory = *me->value.directory;
            const auto new_directory = local_filepath(new_path);

            MSS(!std::filesystem::exists(new_directory), log::stream() << "Error: When renaming " << directory << " into " << new_directory << ": target directory already exists" << std::endl);

            const auto new_content_fp_nonleaf = current_config().content_fp_leaf(new_directory);
            MSS(!std::filesystem::exists(new_content_fp_nonleaf), log::stream() << "Error: When renaming " << directory << " into " << new_content_fp_nonleaf << ": target file already exists" << std::endl);

            if (std::filesystem::exists(directory))
            {
                std::filesystem::rename(directory, new_directory);
            }
            else
            {
                MSS(!!me->value.content_fp);
                std::filesystem::rename(*me->value.content_fp, new_content_fp_nonleaf);
            }

            path_ = new_path;
            MSS(reload_());

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

        bool remove_current()
        {
            MSS_BEGIN(bool);

            const Forest *forest;
            std::size_t ix;
            MSS(get(forest, ix, path_));

            {
                auto current_fp = current_filepath();
                auto trash_fp = trash_filepath(path_);
                log::stream() << "Note: Moving " << current_fp << " to " << trash_fp << std::endl;
                std::filesystem::remove_all(trash_fp);
                std::filesystem::create_directories(trash_fp);
                std::filesystem::rename(current_fp, trash_fp);

                current_fp += ".md";
                trash_fp += ".md";
                if (std::filesystem::exists(current_fp))
                    std::filesystem::rename(current_fp, trash_fp);
            }

            path_.pop_back();
            if (ix+1 < forest->size())
                path_.push_back(forest->nodes[ix+1].value.short_name);
            else if (forest->size() > 1)
                path_.push_back(forest->nodes[ix-1].value.short_name);

            MSS(reload_());

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

        bool get(const Forest *&forest, std::size_t &ix, const Path &path)
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
        bool get(const Node *&node, const Path &path)
        {
            MSS_BEGIN(bool);
            const Forest *forest;
            std::size_t ix;
            MSS(get(forest, ix, path));
            node = &forest->nodes[ix];
            MSS_END();
        }
        bool get_parent(Node *&parent, const Path &path)
        {
            MSS_BEGIN(bool);

            MSS(!path.empty());
            auto parent_path = path;
            parent_path.pop_back();

            Forest *forest;
            std::size_t ix;
            {
                MSS(!!tree_);
                MSS_Q(tree_->find(forest, ix, parent_path));
                assert(!!forest);
            }

            parent = &forest->nodes[ix];

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

        bool save_content_(const Node &node) const
        {
            MSS_BEGIN(bool);
            if (node.value.content_fp)
            {
                std::ofstream fo{*node.value.content_fp};

                fo << "<!--" << std::endl;
                fo << "[proast]";
                if (node.value.my_cost)
                    fo << "(my_cost:" << *node.value.my_cost << ")";
                fo << std::endl;
                fo << "{" << std::endl;
                for (const auto &child: node.childs.nodes)
                {
                    if (child.value.link)
                    {
                        fo << "  [link](path:" << to_string(*child.value.link) << ")" << std::endl;
                    }
                }
                fo << "}" << std::endl;
                fo << "-->" << std::endl;

                std::size_t ix = 0;
                auto ftor = [&](std::size_t line_ix, const std::string &txt, auto style)
                {
                    for (; ix < line_ix; ++ix)
                        fo << std::endl;
                    fo << txt;
                };
                node.value.preview.each(ftor);
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
                            *it++ = node.value.short_name;
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
                Config::create_default(root);

                const std::string name = root.stem().string();
                auto &config = name__config_[name];

                const auto config_fp = Config::filepath(root);
                MSS(config.reload(config_fp), log::stream() << "Error: Could not load the configuration from " << config_fp << std::endl);

                MSS(tree->load(root, config));

                if (path_.empty())
                    path_ = tree->first_root_path();
            }

            tree_.swap(tree);

            if (!load_metadata_())
                log::stream() << "Warning: Could not load metadata" << std::endl;

            MSS_END();
        }

        const std::vector<std::filesystem::path> roots_;

        model::Events *events_{};
        std::map<std::string, Config> name__config_;
        const Config default_config_;
        std::shared_ptr<Tree> tree_;
        Path path_;

        Bookmarks bookmarks_;

        using Clock = std::chrono::high_resolution_clock;
        std::optional<Clock::time_point> save_tp_ = Clock::now();
        Clock::time_point reload_tp_ = Clock::now();
    };

} } 

#endif
