#include <proast/model/Tree.hpp>
#include <proast/model/Markdown.hpp>
#include <proast/log.hpp>
#include <gubg/mss.hpp>
#include <gubg/tree/stream.hpp>
#include <gubg/OnlyOnce.hpp>
#include <gubg/naft/Range.hpp>
#include <gubg/graph/TopologicalOrder.hpp>
#include <map>
#include <list>
#include <vector>
#include <map>
#include <set>
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

        auto each_out_edge = [&](auto nodeptr, auto &&recurse)
        {
            MSS_BEGIN(bool);
            for (auto &child: nodeptr->childs.nodes)
            {
                if (child.value.link)
                {
                    Node *my_node;
                    MSS(find(my_node, *child.value.link));
                    MSS(recurse(my_node));
                }
                else
                {
                    MSS(recurse(&child));
                }
            }
            MSS_END();
        };

        gubg::graph::TopologicalOrder<Node *> topo;
        for (auto &root: root_forest().nodes)
            if (!topo.process(&root, each_out_edge, false))
            {
                log::stream() << "Error: Links and Tree are not a DAG:" << std::endl;
                for (auto nodeptr: topo.cycle)
                    log::stream() << "  " << nodeptr->value.key << std::endl;
                MSS(false);
            }

        //TODO: this is a heavy structure, it can be optimised though. Currently, this grows quadratic wrt node count.
        //With some care, it should be possible to forget reachables once they are not necessary anymore.
        std::map<Node*, std::set<Node*>> node__reachables;
        for (auto nodeptr: topo.order)
        {
            auto &my_reachables = node__reachables[nodeptr];

            //Add myself
            my_reachables.insert(nodeptr);

            //Add all subs
            auto aggregate_reachables = [&](auto subnode)
            {
                auto it = node__reachables.find(subnode);
                if (it != node__reachables.end())
                    for (auto subsubptr: it->second)
                        my_reachables.insert(subsubptr);
                return true;
            };
            MSS(each_out_edge(nodeptr, aggregate_reachables));

            auto &node = *nodeptr;
            node.value.total_cost = 0;
            node.value.done_cost = 0;
            for (auto rnode: my_reachables)
            {
                const auto rcost = rnode->value.my_cost.value_or(0);
                node.value.total_cost += rcost;

                double fraction_done = 0;
                if (rnode->value.state)
                    switch (*rnode->value.state)
                    {
                        case State::Unclear:      fraction_done = 0.0; break;
                        case State::Clear:        fraction_done = 0.1; break;
                        case State::Thinking:     fraction_done = 0.1; break;
                        case State::Designed:     fraction_done = 0.3; break;
                        case State::Implementing: fraction_done = 0.3; break;
                        case State::Done:         fraction_done = 1.0; break;
                    }
                node.value.done_cost += fraction_done*rcost;

                if (nodeptr->value.deadline)
                {
                    nodeptr->value.eta = nodeptr->value.deadline;
                    if (!rnode->value.eta || nodeptr->value.eta < rnode->value.eta)
                        rnode->value.eta = nodeptr->value.eta;
                }
            }
        }

        MSS_END();
    }

    bool Tree::set_paths()
    {
        MSS_BEGIN(bool);
        for (auto &root: root_forest().nodes)
        {
            Path path;
            auto ftor = [&](auto &node, const auto &gpath, unsigned int visit_count)
            {
                if (visit_count == 0)
                {
                    path.push_back(node.value.key);
                    node.value.path = path;
                }
                else
                    path = node.value.path;
            };
            gubg::tree::Path gpath;
            root.dfs(ftor, gpath);
        }
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
