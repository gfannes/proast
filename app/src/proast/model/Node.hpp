#ifndef HEADER_proast_model_Node_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Node_hpp_ALREADY_INCLUDED

#include <proast/model/enums.hpp>
#include <proast/model/Metadata.hpp>
#include <proast/model/StringPath.hpp>
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
        using CPtr = std::shared_ptr<const Node_>;

        static Ptr create(Type);
        static Ptr create(Type, const std::string &name);

        std::filesystem::path segment;
        std::filesystem::path path() const;
        std::string name() const;
        void set_name(const std::string &);

        dto::List::Ptr content;

        Metadata metadata;
        Type type = Type::Virtual;

        WPtr parent;
        WPtr child;
        WPtr up;
        WPtr down;
        WPtr link;
        std::vector<Ptr> childs;

        Ptr append_child(Type);
        Ptr find(const StringPath &);
        template <typename Ftor>
        Ptr find_child(Ftor &&ftor)
        {
            for (auto child: childs)
            {
                if (!child)
                    continue;
                if (ftor(child))
                    return child;
            }
            return Ptr{};
        }

        StringPath to_string_path() const;
        StringPath to_string_path(Ptr &root) const;
        bool to_node_path(std::vector<Ptr> &, const StringPath &);

        std::size_t selected_ix() const;
        bool get_child_ix(std::size_t &child_ix, const Ptr &child) const;

        void clear_dependencies();
        void add_dependencies(Ptr);
        std::size_t dependency_count() const;
        template <typename Ftor>
        void each_dependency(Ftor &&ftor) const
        {
            if (auto self = shared_from_this()->resolve())
                for (auto &wptr: self->all_dependencies_)
                    if (auto ptr = wptr.lock())
                        ftor(ptr);
        }

        double total_effort() const;
        double total_todo() const;
        double total_completion_pct() const;
        Tags all_tags() const;
        std::optional<double> priority() const;

        unsigned int node_count() const;

        CPtr resolve() const;
        Ptr resolve();

    private:
        Node_(Type);

        void append_segment_(std::filesystem::path &) const;
        std::optional<std::string> name_;

        //We cannot keep weak_ptr in a set, but this vector has unique (or nullptr) elements
        using DependenciesVector = std::vector<WPtr>;
        DependenciesVector all_dependencies_;
        static std::set<Ptr> to_set_(const DependenciesVector &);
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
