#ifndef HEADER_proast_dto_List_hpp_ALREADY_INCLUDED
#define HEADER_proast_dto_List_hpp_ALREADY_INCLUDED

#include <vector>
#include <string>
#include <memory>

namespace proast { namespace dto { 
    struct List
    {
        std::vector<std::wstring> items;
        int ix = 0;

        using Ptr = std::shared_ptr<List>;
        static Ptr create(){return Ptr{new List};}
    };
} } 

#endif
