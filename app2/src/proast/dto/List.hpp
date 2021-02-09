#ifndef HEADER_proast_dto_List_hpp_ALREADY_INCLUDED
#define HEADER_proast_dto_List_hpp_ALREADY_INCLUDED

#include <proast/dto/StyleString.hpp>
#include <vector>
#include <string>
#include <memory>

namespace proast { namespace dto { 
    struct List
    {
        StyleString name;
        std::vector<std::string> items;
        int ix = -1;

        using Ptr = std::shared_ptr<List>;
        static Ptr create(){return Ptr{new List};}
    };
} } 

#endif
