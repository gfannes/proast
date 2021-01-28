#ifndef HEADER_proast_Content_hpp_ALREADY_INCLUDED
#define HEADER_proast_Content_hpp_ALREADY_INCLUDED

#include <vector>
#include <string>
#include <memory>
#include <filesystem>

namespace proast { 
    class Content
    {
    public:
        using Ptr = std::unique_ptr<Content>;
        static Ptr create(const std::filesystem::path &path);

        std::vector<std::wstring> lines;
        std::size_t line_ix = 0;
    };
} 

#endif
