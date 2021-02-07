#ifndef HEADER_proast_model_Node_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Node_hpp_ALREADY_INCLUDED

#include <string>
#include <filesystem>
#include <memory>
#include <vector>
#include <optional>

namespace gubg { namespace tree { 
    template <typename Data> class Node;
} } 

namespace proast { namespace model { 
    class Data;

    using Node = gubg::tree::Node<Data>;

    class Node_: public std::enable_shared_from_this<Node_>
    {
    public:
        using Ptr = std::shared_ptr<Node_>;

        static Ptr create();
        static Ptr create(const std::string &);

        std::filesystem::path segment;
        std::filesystem::path path() const;
        std::string name() const;

        Ptr parent;
        std::vector<Ptr> childs;

        Ptr append_child();

        unsigned int node_count() const;

    private:
        void append_segment_(std::filesystem::path &) const;
        std::optional<std::string> name_;
    };

    using Node2 = Node_::Ptr;

    template <typename NodePtr, typename Ftor>
    void depth_first_search(const NodePtr &node, Ftor &&ftor)
    {
        if (!node)
            return;
        for (auto &child: node->childs)
            depth_first_search(child, ftor);
        ftor(node);
    }
} } 

#endif
