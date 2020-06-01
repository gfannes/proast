#include <proast/model/Bookmarks.hpp>
#include <proast/log.hpp>
#include <gubg/naft/Range.hpp>
#include <gubg/file/system.hpp>
#include <gubg/mss.hpp>
#include <fstream>

namespace proast { namespace model { 

    bool Bookmarks::load(const std::filesystem::path &fn)
    {
        MSS_BEGIN(bool);

        std::string content;
        MSS(gubg::file::read(content, fn));

        gubg::naft::Range range0{content};
        while (range0.pop_tag("bookmark"))
        {
            char32_t ch = 0;
            for (const auto &[k,v]: range0.pop_attrs())
            {
                if (false) {}
                else if (k == "char32"){ch = std::stoul(v);}
                else {MSS(false, log::stream() << "Error: Unexpected key " << k << " while parsing bookmarks" << std::endl);}
            }

            gubg::naft::Range range1;
            MSS(range0.pop_block(range1));

            Path path;
            while (range1.pop_tag("segment"))
            {
                for (const auto &[k,v]: range1.pop_attrs())
                {
                    if (false) {}
                    else if (k == "str"){path.push_back(v);}
                    else {MSS(false, log::stream() << "Error: Unexpected key " << k << " while parsing bookmarks" << std::endl);}
                }
            }

            ch__path_[ch] = path;
        }

        MSS_END();
    }
    bool Bookmarks::save(const std::filesystem::path &fn) const
    {
        MSS_BEGIN(bool);
        std::ofstream fo{fn};
        for (const auto &[ch,path]: ch__path_)
        {
            fo << "[bookmark](char32:" << (unsigned int)ch << "){";
            for (const auto &segment: path)
                fo << "[segment](str:" << segment << ")";
            fo << "}" << std::endl;
        }
        MSS_END();
    }

    bool Bookmarks::get(Path &path, char32_t ch) const
    {
        MSS_BEGIN(bool);
        auto it = ch__path_.find(ch);
        MSS_Q(it != ch__path_.end());
        path = it->second;
        MSS_END();
    }
    void Bookmarks::set(char32_t ch, const Path &path)
    {
        ch__path_[ch] = path;
    }

} } 
