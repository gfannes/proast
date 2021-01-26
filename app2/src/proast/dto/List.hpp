#ifndef HEADER_proast_dto_List_hpp_ALREADY_INCLUDED
#define HEADER_proast_dto_List_hpp_ALREADY_INCLUDED

#include <vector>
#include <string>
#include <memory>

namespace proast { namespace dto { 
    struct List
    {
        std::wstring name;
        std::vector<std::wstring> items;
        int ix = -1;

        using Ptr = std::shared_ptr<List>;
        static Ptr create(){return Ptr{new List};}
    };
} } 

#endif
