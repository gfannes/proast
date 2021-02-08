#ifndef HEADER_proast_model_Node_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Node_hpp_ALREADY_INCLUDED

#include <proast/model/Metadata.hpp>
#include <proast/model/Path.hpp>
#include <proast/dto/List.hpp>
#include <string>
#include <filesystem>
#include <memory>
#include <vector>
#include <optional>
#include <set>

namespace proast { namespace model { 
    class Node_: public std::enable_shared_from_this<Node_>
    {
    public:
        using Ptr = std::shared_ptr<Node_>;
        using WPtr = std::weak_ptr<Node_>;

        static Ptr create();
        static Ptr create(const std::string &);

        std::filesystem::path segment;
        std::filesystem::path path() const;
        std::string name() const;

        dto::List::Ptr content;

        Metadata metadata;

        WPtr parent;
        WPtr child;
        WPtr up;
        WPtr down;
        std::vector<Ptr> childs;

        Ptr append_child();
        Ptr find(const Path &);

        Path to_path() const;
        Path to_path(Ptr &root) const;

        void clear_dependencies();
        void add_dependencies(Ptr);
        std::size_t dependency_count() const;
        template <typename Ftor>
        void each_dependency(Ftor &&ftor) const
        {
            for (auto &wptr: all_dependencies_)
                if (auto ptr = wptr.lock())
                    ftor(ptr);
        }

        double total_effort() const;
        Tags all_tags() const;

        unsigned int node_count() const;

    private:
        void append_segment_(std::filesystem::path &) const;
        std::optional<std::string> name_;

        //We cannot keep weak_ptr in a set, but this vector has unique (or nullptr) elements
        std::vector<WPtr> all_dependencies_;
        static std::set<Ptr> to_set_(std::vector<WPtr> &);
    };

    using Node = Node_::Ptr;

    //Will only call ftor for valid nodes
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
