#ifndef HEADER_proast_model_Data_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Data_hpp_ALREADY_INCLUDED

#include <proast/model/Node.hpp>
#include <proast/dto/List.hpp>
#include <proast/model/Metadata.hpp>
#include <filesystem>

namespace proast { namespace model { 
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
        std::string name;
        std::filesystem::path path;
        dto::List::Ptr content;
        Metadata metadata;
    private:
    };
} } 

#endif
