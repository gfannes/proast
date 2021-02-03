#ifndef HEADER_proast_model_Data_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Data_hpp_ALREADY_INCLUDED

#include <proast/dto/List.hpp>
#include <proast/model/Metadata.hpp>
#include <filesystem>

namespace gubg { namespace tree { 
    template <typename Data> class Node;
} } 

namespace proast { namespace model { 
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
        dto::List::Ptr content;
        Metadata metadata;
    private:
    };
} } 

#endif
