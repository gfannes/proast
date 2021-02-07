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

    //Privates
    void Node_::append_segment_(std::filesystem::path &path) const
    {
        if (parent)
            parent->append_segment_(path);
        path /= segment;
    }
} } 
