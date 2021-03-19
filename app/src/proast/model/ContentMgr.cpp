#include <proast/model/ContentMgr.hpp>
#include <fstream>
#include <string>

namespace proast { namespace model {
    ContentMgr::ContentMgr()
    {
        allowed_extensions_ = {".md", ".h", ".c", ".hpp", ".cpp", ".txt", ".naft", ".tsv", ".rb", ".chai", ".log"};
    }

    dto::List::Ptr ContentMgr::load(const std::filesystem::path &path, bool quiet) const
    {
        dto::List::Ptr ptr;
        if (allowed_extensions_.count(path.extension().string()))
        {
            std::ifstream fi{path};
            for (std::string line; std::getline(fi, line);)
            {
                if (!ptr)
                    ptr = dto::List::create();
                //TODO: Convert tab into double-space
                ptr->items.emplace_back(line);
                ptr->items.back().ix__attention[0] = 1;
            }
        }
        else if (!quiet)
        {
            if (!ptr)
                ptr = dto::List::create();
            //TODO: The ContentMgr should not add this info, this is upto the Presenter
            ptr->items.emplace_back("This extension is not supported");
            ptr->items.back().ix__attention[0] = 6;
        }
        if (ptr)
            ptr->ix = 0;
        return ptr;
    }
} }
