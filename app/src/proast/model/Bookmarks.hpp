#ifndef HEADER_proast_model_Bookmarks_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Bookmarks_hpp_ALREADY_INCLUDED

#include <proast/model/Path.hpp>
#include <filesystem>
#include <map>

namespace proast { namespace model { 

    class Bookmarks
    {
    public:
        bool load(const std::filesystem::path &fn);
        bool save(const std::filesystem::path &fn) const;

        bool get(Path &, char32_t) const;
        void set(char32_t, const Path &);

    private:
        std::map<char32_t, Path> ch__path_;
    };

} } 

#endif
