#ifndef HEADER_proast_model_Bookmarks_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Bookmarks_hpp_ALREADY_INCLUDED

#include <proast/model/Path.hpp>
#include <gubg/naft/Node.hpp>
#include <gubg/naft/Range.hpp>
#include <filesystem>
#include <map>

namespace proast { namespace model { 

    class Bookmarks
    {
    public:
        bool save(const std::filesystem::path &fn) const;
        bool load(const std::filesystem::path &fn);

        bool get(Path &, char) const;
        void set(char, const Path &);

        template <typename Ftor>
        void each(Ftor &&ftor)
        {
            for (auto &[ch, path]: ch__path_)
                ftor(ch, path);
        }

    private:
        bool stream_(gubg::naft::Node &) const;
        bool parse_(gubg::naft::Range &);

        std::map<char, Path> ch__path_;
    };

} } 

#endif
