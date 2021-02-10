#include <proast/model/Node.hpp>

namespace proast { namespace model { 
    Node_::Ptr Node_::create()
    {
        return Ptr{new Node_};
    }
    Node_::Ptr Node_::create(const std::string &str)
    {
        auto ptr = create();
        ptr->name_ = str;
        return ptr;
    }
    Node_::Ptr Node_::append_child()
    {
        auto ptr = create();
        ptr->parent = shared_from_this();
        childs.push_back(ptr);
        return ptr;
    }
    std::filesystem::path Node_::path() const
    {
        std::filesystem::path path;
        append_segment_(path);
        return path;
    }
    std::string Node_::name() const
    {
        if (name_)
            return *name_;
        return segment.filename().string();
    }
    unsigned int Node_::node_count() const
    {
        unsigned int count = 0;
        depth_first_search(shared_from_this(), [&](auto &){count += 1;});
        return count;
    }
    Node Node_::find(const Path &path)
    {
        auto node = shared_from_this();
        for (const auto &part: path)
        {
            auto &childs = node->childs;
            node.reset();
            for (auto &child: childs)
                if (child && child->name() == part)
                {
                    node = child;
                    break;
                }
            if (!node)
                break;
        }
        return node;
    }

    void Node_::clear_dependencies()
    {
        all_dependencies_.clear();
    }
    void Node_::add_dependencies(Ptr other)
    {
        if (!other)
            return;

        auto my_set = to_set_(all_dependencies_);
        auto other_set = to_set_(other->all_dependencies_);

        my_set.insert(other);
        my_set.insert(other_set.begin(), other_set.end());

        all_dependencies_.resize(my_set.size());
        std::copy(my_set.begin(), my_set.end(), all_dependencies_.begin());
    }
    std::size_t Node_::dependency_count() const
    {
        return all_dependencies_.size();
    }
    Path Node_::to_path() const
    {
        Path p;
        for (auto node = shared_from_this(); node; )
        {
            auto parent = node->parent.lock();
            if (!parent)
                //We do not include the root node name
                break;
            p.push_back(node->name());
            node = parent;
        }
        std::reverse(p.begin(), p.end());
        return p;
    }
    Path Node_::to_path(Ptr &root) const
    {
        Path p;
        for (auto node = shared_from_this(); node; )
        {
            auto parent = node->parent.lock();
            if (!parent)
                //We do not include the root node name
                break;
            if (node == root)
                //We are at a pseudo-root and stop as well
                break;
            p.push_back(node->name());
            node = parent;
        }
        std::reverse(p.begin(), p.end());
        return p;
    }

    double Node_::total_effort() const
    {
        double sum = metadata.effort.value_or(0.0);
        for (auto &wptr: all_dependencies_)
            if (auto ptr = wptr.lock())
                sum += ptr->metadata.effort.value_or(0.0);
        return sum;
    }
    Tags Node_::all_tags() const
    {
        Tags tags = metadata.tags;
        for (auto &wptr: all_dependencies_)
            if (auto ptr = wptr.lock())
                tags.insert(ptr->metadata.tags.begin(), ptr->metadata.tags.end());
        return tags;
    }

    //Privates
    void Node_::append_segment_(std::filesystem::path &path) const
    {
        if (auto p = parent.lock())
            p->append_segment_(path);
        path /= segment;
    }
    std::set<Node> Node_::to_set_(std::vector<WPtr> &vec)
    {
        std::set<Ptr> set;
        for (auto &wptr: vec)
            if (auto ptr = wptr.lock())
                set.insert(ptr);
        return set;
    }
} } 