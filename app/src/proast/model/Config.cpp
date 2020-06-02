#include <proast/model/Config.hpp>
#include <gubg/naft/Range.hpp>
#include <gubg/mss.hpp>
#include <fstream>

namespace proast { namespace model { 

    std::filesystem::path Config::filepath(const std::filesystem::path &root)
    {
        return root/".proast.config";
    }

    bool Config::create_default(const std::filesystem::path &root)
    {
        MSS_BEGIN(bool);

        //Create .proast.config if it does not exist yet
        {
            const auto fp = filepath(root);
            if (!std::filesystem::exists(fp))
            {
                std::ofstream fo{fp};
                //TODO: set default values to give an indication to the user what exists
            }
        }

        MSS_END();
    }

    std::filesystem::path Config::content_fp_leaf(const std::filesystem::path &directory) const
    {
        auto fp = directory;
        fp += extension();
        return fp;
    }
    std::filesystem::path Config::content_fp_nonleaf(const std::filesystem::path &directory) const
    {
        return directory / index_filename();
    }

    bool Config::reload(const std::filesystem::path &fp)
    {
        MSS_BEGIN(bool);

        std::string content;
        MSS(gubg::file::read(content, fp));

        gubg::naft::Range range{content};

        MSS_END();
    }
} } 
