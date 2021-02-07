#include <proast/model/Bookmarks.hpp>
#include <proast/log.hpp>
#include <proast/util.hpp>
#include <gubg/naft/Document.hpp>
#include <gubg/file/system.hpp>
#include <gubg/mss.hpp>
#include <fstream>

namespace proast { namespace model { 

    bool Bookmarks::save(const std::filesystem::path &fn) const
    {
        MSS_BEGIN(bool);

        std::ofstream fo{fn};
        
        gubg::naft::Document doc{fo};
        MSS(stream_(doc));

        MSS_END();
    }
    bool Bookmarks::load(const std::filesystem::path &fn)
    {
        MSS_BEGIN(bool);

        std::string content;
        MSS(gubg::file::read(content, fn));

        gubg::naft::Range range{content};

        MSS(parse_(range));

        MSS_END();
    }

    bool Bookmarks::get(Path &path, char ch) const
    {
        MSS_BEGIN(bool);
        auto it = ch__path_.find(ch);
        MSS_Q(it != ch__path_.end());
        path = it->second;
        MSS_END();
    }
    void Bookmarks::set(char ch, const Path &path)
    {
        ch__path_[ch] = path;
    }

    //Privates
    bool Bookmarks::stream_(gubg::naft::Node &parent) const
    {
        MSS_BEGIN(bool);

        for (const auto &[ch,path]: ch__path_)
        {
            auto n = parent.node("Bookmark");
            n.attr("char", (unsigned int)ch);
            n.attr("path", to_string(path));
        }

        MSS_END();
    }
    bool Bookmarks::parse_(gubg::naft::Range &range0)
    {
        MSS_BEGIN(bool);

        while (range0.pop_tag("Bookmark"))
        {
            std::string key, value;

            MSS(range0.pop_attr(key, value));
            MSS(key == "char");
            const auto ch = (char)std::stoi(value);

            MSS(range0.pop_attr(key, value));
            MSS(key == "path");
            const auto path = to_path(value);

            ch__path_[ch] = path;
        }

        MSS_END();
    }

} } 
