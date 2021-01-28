#ifndef HEADER_proast_Data_hpp_ALREADY_INCLUDED
#define HEADER_proast_Data_hpp_ALREADY_INCLUDED

#include <proast/Content.hpp>
#include <filesystem>

namespace gubg { namespace tree { 
    template <typename Data> class Node;
} } 

namespace proast { 
    class Data;

    using Node = gubg::tree::Node<Data>;

    struct Navigation
    {
        Node *parent{};
        Node *child{};
        Node *up{};
        Node *down{};
    };

    class Data
    {
    public:
        Navigation navigation;
        std::wstring name;
        std::filesystem::path path;
        Content::Ptr content;
    private:
    };
} 

#endif
