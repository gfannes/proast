#ifndef HEADER_proast_model_Model_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Model_hpp_ALREADY_INCLUDED

#include <proast/model/Events.hpp>
#include <proast/model/Tree.hpp>
#include <gubg/file/system.hpp>
#include <gubg/naft/Range.hpp>
#include <gubg/OnlyOnce.hpp>
#include <gubg/mss.hpp>
#include <optional>
#include <vector>
#include <chrono>
#include <fstream>
#include <cassert>

namespace proast { namespace model { 

    enum class Mode {Develop, Rework, Nr_};

    class Model
    {
    public:
        void set_events_dst(model::Events *events)
        {
            events_ = events;
            if (events_)
                events_->message("Events destination was set");
        }

        Mode mode = Mode::Develop;
        void set_mode(Mode m)
        {
            const auto do_notify = (m != mode);
            mode = m;
            if (do_notify)
                events_->notify();
        }

        bool operator()()
        {
            MSS_BEGIN(bool);

            if (!tree_)
            {
                std::filesystem::path root;
                MSS(Tree::find_root_filepath(root, std::filesystem::current_path()));
                tree_.emplace();
                MSS(tree_->load(root));
                MSS(load_metadata_());
                path_ = tree_->root_path();
            }

            const auto now = Clock::now();
            if (now >= save_tp_)
            {
                //Save from time to time
                MSS(save_metadata_());
                save_tp_ = now+std::chrono::milliseconds(300);
            }

            MSS_END();
        }
        std::filesystem::path root_filepath() const
        {
            if (!!tree_)
                return tree_->root_filepath();
            return std::filesystem::path{};
        }

        const Path &path() const {return path_;}
        void set_path(const Path &path)
        {
            if (path.empty())
            {
                if (!!tree_)
                    path_ = tree_->root_path();
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
                MSS(tree_->find(forest, ix, parent_path));
                assert(!!forest);
            }

            parent = &forest->nodes[ix];

            MSS_END();
        }

    private:
        std::filesystem::path metadata_fn_() const
        {
            std::filesystem::path fn;
            if (tree_)
                fn = tree_->root_filepath() / ".proast/metadata.txt";
            return fn;
        }
        bool save_metadata_()
        {
            MSS_BEGIN(bool);

            MSS(!!tree_);

            std::ofstream fo{metadata_fn_()};

            Path path;
            auto ftor = [&](auto &node, const auto &int_path, auto visit_count)
            {
                if (visit_count == 0)
                {
                    path.push_back(node.value.short_name);
                    fo << '[';
                    gubg::OnlyOnce skip_separator;
                    for (const auto &segment: path)
                    {
                        if (!skip_separator())
                            fo << ':';
                        fo << segment;
                    }
                    fo << ']';
                    fo << "(active_ix:" << node.value.active_ix << ")";
                    fo << std::endl;
                }
                else
                {
                    assert(!path.empty());
                    MSS(!path.empty());
                    path.pop_back();
                }
                return true;
            };
            tree_->root_forest().dfs(ftor);

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

                auto it = attrs.find("active_ix");
                if (it != attrs.end())
                {
                    Node *node;
                    if (tree_->find(node, path))
                        node->value.active_ix = std::stoul(it->second);
                }
            }

            MSS_END();
        }

        model::Events *events_{};
        std::optional<Tree> tree_;
        Path path_;

        using Clock = std::chrono::high_resolution_clock;
        Clock::time_point save_tp_ = Clock::now();
    };

} } 

#endif
