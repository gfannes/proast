#include <proast/Content.hpp>
#include <proast/util.hpp>
#include <fstream>
#include <set>

namespace proast { 
    Content::Ptr Content::create(const std::filesystem::path &path)
    {
        Ptr ptr{new Content};
        static std::set<std::string> allowed_extensions = {".md", ".h", ".c", ".hpp", ".cpp", ".txt", ".naft" };
        if (allowed_extensions.count(path.extension().string()))
        {
            std::ifstream fi{path};
            for (std::string line; std::getline(fi, line);)
                //TODO: Do proper conversion to unicode, otherwise, ftxui might hang
                ptr->lines.push_back(to_wstring(line));
        }
        else
        {
            ptr->lines.push_back(L"This extension is not supported");
        }
        return ptr;
    }
} 
