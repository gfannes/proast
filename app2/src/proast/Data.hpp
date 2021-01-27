#ifndef HEADER_proast_Data_hpp_ALREADY_INCLUDED
#define HEADER_proast_Data_hpp_ALREADY_INCLUDED

#include <filesystem>

namespace proast { 
    class Data
    {
    public:
        std::wstring name;
        std::wstring selected;
        std::filesystem::path path;
        std::size_t line_ix = 0;
    private:
    };
} 

#endif
