#include <proast/model/Bookmarks.hpp>
#include <proast/log.hpp>
#include <proast/util.hpp>
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
            wchar_t ch = 0;
            for (const auto &[k,v]: range0.pop_attrs())
            {
                if (false) {}
                else if (k == "wchar_t"){ch = std::stoul(v);}
                else {MSS(false, log::ostream() << "Error: Unexpected key " << k << " while parsing bookmarks" << std::endl);}
            }

            gubg::naft::Range range1;
            MSS(range0.pop_block(range1));

            Path path;
            while (range1.pop_tag("segment"))
            {
                for (const auto &[k,v]: range1.pop_attrs())
                {
                    if (false) {}
                    else if (k == "str"){path.push_back(proast::to_wstring(v));}
                    else {MSS(false, log::ostream() << "Error: Unexpected key " << k << " while parsing bookmarks" << std::endl);}
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
            fo << "[bookmark](wchar_t:" << (unsigned int)ch << "){";
            for (const auto &segment: path)
                fo << "[segment](str:" << proast::to_string(segment) << ")";
            fo << "}" << std::endl;
        }
        MSS_END();
    }

    bool Bookmarks::get(Path &path, wchar_t ch) const
    {
        MSS_BEGIN(bool);
        auto it = ch__path_.find(ch);
        MSS_Q(it != ch__path_.end());
        path = it->second;
        MSS_END();
    }
    void Bookmarks::set(wchar_t ch, const Path &path)
    {
        ch__path_[ch] = path;
    }

} } 
