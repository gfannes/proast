#include <proast/model/Content.hpp>
#include <proast/util.hpp>
#include <fstream>

namespace proast { namespace model {
    Content::Content()
    {
        allowed_extensions_ = {".md", ".h", ".c", ".hpp", ".cpp", ".txt", ".naft" };
    }

    dto::List::Ptr Content::create(const std::filesystem::path &path) const
    {
        auto ptr = dto::List::create();
        if (allowed_extensions_.count(path.extension().string()))
        {
            std::ifstream fi{path};
            for (std::string line; std::getline(fi, line);)
                //TODO: Do proper conversion to unicode, otherwise, ftxui might hang
                ptr->items.push_back(to_wstring(line));
        }
        else
        {
            ptr->items.push_back(L"This extension is not supported");
        }
        ptr->ix = 0;
        return ptr;
    }
} }
