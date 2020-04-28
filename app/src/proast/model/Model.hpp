#ifndef HEADER_proast_model_Model_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Model_hpp_ALREADY_INCLUDED

#include <proast/model/Events.hpp>
#include <proast/model/Tree.hpp>
#include <gubg/mss.hpp>
#include <optional>
#include <vector>

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
                MSS(Tree::find_root(root, std::filesystem::current_path()));
                tree_.emplace();
                MSS(tree_->load(root));
                std::cout << *tree_ << std::endl;
            }
            MSS_END();
        }
        std::filesystem::path root_path() const
        {
            if (!!tree_)
                return tree_->root_path();
            return std::filesystem::path{};
        }

        bool get_me(std::vector<std::string> &ary)
        {
            MSS_BEGIN(bool);

            MSS(!!tree_);
            auto me_node = tree_->find(path_);
            MSS(!!me_node);

            ary.resize(0);
            for (const auto &n: me_node->childs.nodes)
                ary.push_back(n.value.short_name);

            MSS_END();
        }

    private:
        model::Events *events_{};
        std::optional<Tree> tree_;
        Path path_;
    };

} } 

#endif
