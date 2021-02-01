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

        bool get(Path &, wchar_t) const;
        void set(wchar_t, const Path &);

        template <typename Ftor>
        void each(Ftor &&ftor)
        {
            for (auto &[wchar, path]: ch__path_)
                ftor(wchar, path);
        }

    private:
        std::map<wchar_t, Path> ch__path_;
    };

} } 

#endif
