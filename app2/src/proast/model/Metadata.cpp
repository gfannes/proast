#include <proast/model/Metadata.hpp>
#include <gubg/file/system.hpp>
#include <gubg/mss.hpp>

namespace proast { namespace model { 
    void Metadata::clear()
    {
        *this = Metadata{};
    }
    bool Metadata::load(const std::filesystem::path &path)
    {
        MSS_BEGIN(bool);
        if (!std::filesystem::is_regular_file(path))
        {
            clear();
            return true;
        }
        std::string content;
        MSS(gubg::file::read(content, path));
        MSS_END();
    }
    bool Metadata::save(const std::filesystem::path &path) const
    {
        MSS_BEGIN(bool);
        MSS_END();
    }
} } 
