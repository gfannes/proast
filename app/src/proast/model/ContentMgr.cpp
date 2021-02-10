#include <proast/model/ContentMgr.hpp>
#include <fstream>
#include <string>

namespace proast { namespace model {
    ContentMgr::ContentMgr()
    {
        allowed_extensions_ = {".md", ".h", ".c", ".hpp", ".cpp", ".txt", ".naft", ".tsv", ".rb", ".chai"};
    }

    dto::List::Ptr ContentMgr::load(const std::filesystem::path &path) const
    {
        auto ptr = dto::List::create();
        if (allowed_extensions_.count(path.extension().string()))
        {
            std::ifstream fi{path};
            for (std::string line; std::getline(fi, line);)
            {
                //TODO: Convert tab into double-space
                ptr->items.emplace_back(line);
                ptr->items.back().ix__attention[0] = 1;
            }
        }
        else
        {
            ptr->items.emplace_back("This extension is not supported");
            ptr->items.back().ix__attention[0] = 6;
        }
        ptr->ix = 0;
        return ptr;
    }
} }
