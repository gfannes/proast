#include <proast/model/Tree.hpp>
#include <proast/model/Markdown.hpp>
#include <proast/log.hpp>
#include <gubg/mss.hpp>
#include <gubg/tree/stream.hpp>
#include <gubg/OnlyOnce.hpp>
#include <gubg/naft/Range.hpp>
#include <map>
#include <list>
#include <vector>
#include <fstream>
#include <sstream>
#include <cassert>

namespace proast { namespace model { 

    bool Tree::load(const std::filesystem::path &root, const Config &cfg)
    {
        MSS_BEGIN(bool);

        const std::string name = root.stem().string();
        auto it = name__cfg_.find(name);
        MSS(it == name__cfg_.end());

        name__cfg_[name] = cfg;

        //TODO: not sure why we first clear() only set it later
        name__root_filepath_[name].clear();

        auto &node = root_forest_.append();
        MSS(markdown::read_directory(node, root, cfg));

        name__root_filepath_[name] = root;

        MSS_END();
    }

    std::filesystem::path Tree::root_filepath(const std::string &name) const
    {
        std::filesystem::path fp;
        auto it = name__root_filepath_.find(name);
        if (it != name__root_filepath_.end())
            fp = it->second;
        return fp;
    }

    Path Tree::first_root_path() const
    {
        Path path;
        if (!root_forest_.empty())
            path.emplace_back(root_forest_.nodes[0].value.key);
        return path;
    }

    bool Tree::find(Forest *&forest, std::size_t &ix, const Path &path)
    {
        MSS_BEGIN(bool);

        MSS_Q(!path.empty());

        Forest *my_forest = nullptr;
        std::size_t my_ix = 0;
        for (const auto &segment: path)
        {
            //Set/update the my_forest where to look for "segment"
            if (!my_forest)
            {
                my_forest = &root_forest_;
            }
            else
            {
                assert(my_ix < my_forest->size());
                my_forest = &my_forest->nodes[my_ix].childs;
            }

            //Search the my_forest for "segment"
            const auto nr_childs = my_forest->size();
            for (my_ix = 0; my_ix < nr_childs; ++my_ix)
                if (my_forest->nodes[my_ix].value.key == segment)
                    break;
            MSS_Q(my_ix < my_forest->size());
        }

        forest = my_forest;
        ix = my_ix;

        MSS_END();
    }
    bool Tree::find(Node *&node, const Path &path)
    {
        MSS_BEGIN(bool);

        MSS_Q(!path.empty());

        Forest *my_forest = nullptr;
        std::size_t my_ix = 0;
        for (const auto &segment: path)
        {
            //Set/update the my_forest where to look for "segment"
            if (!my_forest)
            {
                my_forest = &root_forest_;
            }
            else
            {
                assert(my_ix < my_forest->size());
                my_forest = &my_forest->nodes[my_ix].childs;
            }

            //Search the my_forest for "segment"
            const auto nr_childs = my_forest->size();
            for (my_ix = 0; my_ix < nr_childs; ++my_ix)
                if (my_forest->nodes[my_ix].value.key == segment)
                    break;
            MSS_Q(my_ix < my_forest->size());
        }

        node = &my_forest->nodes[my_ix];

        MSS_END();
    }
    bool Tree::find(NodeIXPath &nixpath, const Path &path)
    {
        MSS_BEGIN(bool);

        const auto depth = path.size();

        nixpath.resize(depth);
        nixpath.resize(0);

        MSS_Q(depth > 0);

        Forest *my_forest = nullptr;
        std::size_t my_ix = 0;
        for (const auto &segment: path)
        {
            //Set/update the my_forest where to look for "segment"
            if (!my_forest)
            {
                my_forest = &root_forest_;
            }
            else
            {
                assert(my_ix < my_forest->size());
                my_forest = &my_forest->nodes[my_ix].childs;
            }

            //Search the my_forest for "segment"
            const auto nr_childs = my_forest->size();
            for (my_ix = 0; my_ix < nr_childs; ++my_ix)
                if (my_forest->nodes[my_ix].value.key == segment)
                    break;
            MSS_Q(my_ix < my_forest->size());

            nixpath.emplace_back(&my_forest->nodes[my_ix], my_ix);
        }

        MSS_END();
    }
    bool Tree::find(ConstNodeIXPath &cnixpath, const Path &path) const
    {
        MSS_BEGIN(bool);

        const auto depth = path.size();

        cnixpath.resize(depth);
        cnixpath.resize(0);

        MSS_Q(depth > 0);

        const Forest *my_forest = nullptr;
        std::size_t my_ix = 0;
        for (const auto &segment: path)
        {
            //Set/update the my_forest where to look for "segment"
            if (!my_forest)
            {
                my_forest = &root_forest_;
            }
            else
            {
                assert(my_ix < my_forest->size());
                my_forest = &my_forest->nodes[my_ix].childs;
            }

            //Search the my_forest for "segment"
            const auto nr_childs = my_forest->size();
            for (my_ix = 0; my_ix < nr_childs; ++my_ix)
                if (my_forest->nodes[my_ix].value.key == segment)
                    break;
            MSS_Q(my_ix < my_forest->size());

            cnixpath.emplace_back(&my_forest->nodes[my_ix], my_ix);
        }

        MSS_END();
    }

    bool Tree::compute_aggregates()
    {
        MSS_BEGIN(bool);

        std::vector<double> total_stack;
        std::vector<double> done_stack;
        auto ftor = [&](auto &node, const auto &path, unsigned int visit_count)
        {
            double fraction_done = 0;
            if (node.value.state)
                switch (*node.value.state)
                {
                    case State::Unclear:      fraction_done = 0.0; break;
                    case State::Clear:        fraction_done = 0.1; break;
                    case State::Thinking:     fraction_done = 0.1; break;
                    case State::Designed:     fraction_done = 0.3; break;
                    case State::Implementing: fraction_done = 0.3; break;
                    case State::Done:         fraction_done = 1.0; break;
                }
            if (visit_count == 0)
            {
                const auto my_cost = node.value.my_cost.value_or(0);
                total_stack.push_back(my_cost);
                done_stack.push_back(fraction_done*my_cost);
                node.value.total_cost = total_stack.back();
                node.value.done_cost = done_stack.back();
            }
            else
            {
                {
                    const auto child_cost = total_stack.back();
                    node.value.total_cost += child_cost;
                    total_stack.pop_back();
                    total_stack.back() = node.value.total_cost;
                }
                {
                    const auto child_cost = done_stack.back();
                    node.value.done_cost += child_cost;
                    done_stack.pop_back();
                    done_stack.back() = node.value.done_cost;
                }
            }
        };
        root_forest_.dfs(ftor);

        MSS_END();
    }

    void Tree::stream(std::ostream &os) const
    {
        auto ftor = [](std::ostream &os, const Node &n)
        {
            os << n.value.key << std::endl;
        };
        gubg::tree::stream(os, root_forest_, ftor);
    }

} } 
