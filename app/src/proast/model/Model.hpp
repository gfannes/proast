#ifndef HEADER_proast_model_Model_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Model_hpp_ALREADY_INCLUDED

#include <proast/model/Events.hpp>
#include <proast/model/Tree.hpp>
#include <gubg/mss.hpp>
#include <optional>

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

    private:
        model::Events *events_{};
        std::optional<Tree> tree_;
    };

} } 

#endif
