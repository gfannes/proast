#ifndef HEADER_proast_model_Dependencies_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Dependencies_hpp_ALREADY_INCLUDED

#include <proast/model/Node.hpp>
#include <set>

namespace proast { namespace model { 
    class Dependencies
    {
    public:
        void clear() {*this = Dependencies{};}
        std::size_t size() const {return all_nodes_.size();}

        void add(Node *node)                   {all_nodes_.insert(node);}
        void add(const Dependencies &dependee) {all_nodes_.insert(dependee.all_nodes_.begin(), dependee.all_nodes_.end());}

        template <typename Ftor>
        void each(Ftor &&ftor) const
        {
            for (Node *n: all_nodes_)
                ftor(*n);
        }

    private:
        std::set<Node *> all_nodes_;
    };
} } 

#endif
