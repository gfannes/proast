#ifndef HEADER_proast_model_Model_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Model_hpp_ALREADY_INCLUDED

#include <proast/model/Events.hpp>
#include <proast/model/Tree.hpp>
#include <gubg/mss.hpp>
#include <optional>
#include <vector>
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
                path_ = tree_->root_path();
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
        model::Events *events_{};
        std::optional<Tree> tree_;
        Path path_;
    };

} } 

#endif
