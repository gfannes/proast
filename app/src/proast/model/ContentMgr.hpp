#ifndef HEADER_proast_model_ContentMgr_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_ContentMgr_hpp_ALREADY_INCLUDED

#include <proast/dto/List.hpp>
#include <filesystem>
#include <set>
#include <string>

namespace proast { namespace model {
    class ContentMgr
    { 
    public:
        ContentMgr();

        dto::List::Ptr load(const std::filesystem::path &path) const;

    private:
        std::set<std::string> allowed_extensions_;
    };
} } 

#endif
