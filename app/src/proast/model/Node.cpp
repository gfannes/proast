#include <proast/model/Node.hpp>
#include <proast/log.hpp>
#include <gubg/mss.hpp>
#include <cmath>

namespace proast { namespace model { 
    Node_::Node_(Type t): type(t)
    {
    }
    Node_::Ptr Node_::create(Type t)
    {
        return Ptr{new Node_{t}};
    }
    Node_::Ptr Node_::create(Type t, const std::string &name)
    {
        auto ptr = create(t);
        ptr->name_ = name;
        return ptr;
    }

    Node_::Ptr Node_::append_child(Type t)
    {
        auto ptr = create(t);
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
        switch (type)
        {
            case Type::File:
            case Type::Directory:
                return segment.filename().string();
                break;
            case Type::Virtual:
                return "<unnamed virtual>";
                break;
            case Type::Link:
                if (auto l = link.lock())
                    return std::string("=>")+l->name();
                else
                    return "<unlinked link>";
                break;
        }
        return "";
    }
    void Node_::set_name(const std::string &name)
    {
        name_ = name;
    }
    unsigned int Node_::node_count() const
    {
        unsigned int count = 0;
        depth_first_search(shared_from_this(), [&](auto &){count += 1;});
        return count;
    }
    Node Node_::find(const StringPath &path)
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
        auto resolved_other = other->resolve();
        if (!resolved_other)
            return;

        auto my_set = to_set_(all_dependencies_);
        auto other_set = to_set_(resolved_other->all_dependencies_);

        my_set.insert(resolved_other);
        my_set.insert(other_set.begin(), other_set.end());

        all_dependencies_.resize(my_set.size());
        std::copy(my_set.begin(), my_set.end(), all_dependencies_.begin());
    }
    std::size_t Node_::dependency_count() const
    {
        if (auto self = shared_from_this()->resolve())
            return self->all_dependencies_.size();
        return 0;
    }
    StringPath Node_::to_string_path() const
    {
        StringPath p;
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
    StringPath Node_::to_string_path(Ptr &root) const
    {
        StringPath p;
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
    bool Node_::to_node_path(std::vector<Ptr> &node_path, const StringPath &string_path)
    {
        MSS_BEGIN(bool);

        node_path = {shared_from_this()};

        for (const auto &name: string_path)
        {
            auto &childs = node_path.back()->childs;
            auto it = std::find_if(childs.begin(), childs.end(), [&](auto &child){return child->name() == name;});
            MSS(it != childs.end());
            node_path.push_back(*it);
        }

        MSS_END();
    }

    std::size_t Node_::selected_ix() const
    {
        if (auto ch = child.lock())
            for (auto ix = 0u; ix < childs.size(); ++ix)
                if (ch == childs[ix])
                    return ix;
        return 0;
    }

    bool Node_::get_child_ix(std::size_t &child_ix, const Ptr &child) const
    {
        for (auto ix = 0u; ix < childs.size(); ++ix)
            if (childs[ix] == child)
            {
                child_ix = ix;
                return true;
            }
        return false;
    }

    double Node_::total_effort() const
    {
        double sum = 0.0;
        if (auto self = shared_from_this()->resolve())
        {
            sum = self->metadata.effort.value_or(0.0);
            for (auto &wptr: self->all_dependencies_)
                if (auto ptr = wptr.lock())
                    sum += ptr->metadata.effort.value_or(0.0);
        }
        return sum;
    }
    double Node_::total_todo() const
    {
        double sum = 0.0;
        if (auto self = shared_from_this()->resolve())
        {
            sum = self->metadata.get_todo();
            for (auto &wptr: self->all_dependencies_)
                if (auto ptr = wptr.lock())
                    sum += ptr->metadata.get_todo();
        }
        return sum;
    }
    std::optional<double> Node_::priority() const
    {
        std::optional<double> prio;
        if (auto effort = metadata.effort)
        {
            double overdue_days = -7.0;
            double total_volume_db = metadata.get_volume_db() + 6.0*std::max(overdue_days+7, 0.0);
            prio = metadata.get_age()*std::pow(10.0, total_volume_db/20.0)*metadata.get_impact()/(*effort);
        }
        return prio;
    }

    Tags Node_::all_tags() const
    {
        Tags tags;
        if (auto self = shared_from_this()->resolve())
        {
            tags = self->metadata.tags;
            for (auto &wptr: self->all_dependencies_)
                if (auto ptr = wptr.lock())
                    tags.insert(ptr->metadata.tags.begin(), ptr->metadata.tags.end());
        }
        return tags;
    }

    Node_::Ptr Node_::resolve()
    {
        if (type != Type::Link)
            return shared_from_this();
        if (auto lnk = link.lock())
            return lnk->resolve();
        return Ptr{};
    }
    Node_::CPtr Node_::resolve() const
    {
        if (type != Type::Link)
            return shared_from_this();
        if (auto lnk = link.lock())
            return lnk->resolve();
        return CPtr{};
    }

    //Privates
    void Node_::append_segment_(std::filesystem::path &path) const
    {
        if (auto p = parent.lock())
            p->append_segment_(path);
        path /= segment;
    }
    std::set<Node> Node_::to_set_(const DependenciesVector &vec)
    {
        std::set<Ptr> set;
        for (auto &wptr: vec)
            if (auto ptr = wptr.lock())
                set.insert(ptr);
        return set;
    }
} } 
