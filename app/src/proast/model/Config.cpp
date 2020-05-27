#include <proast/model/Config.hpp>
#include <gubg/naft/Range.hpp>
#include <gubg/mss.hpp>
#include <fstream>

namespace proast { namespace model { 

    std::filesystem::path Config::directory(const std::filesystem::path &root)
    {
        return root/".proast";
    }
    std::filesystem::path Config::filepath(const std::filesystem::path &root)
    {
        return Config::directory(root)/"config.naft";
    }

    bool Config::create_default(const std::filesystem::path &root)
    {
        MSS_BEGIN(bool);

        const auto dot_proast = Config::directory(root);
        if (!std::filesystem::exists(dot_proast))
            std::filesystem::create_directories(dot_proast);
        MSS(!!std::filesystem::exists(dot_proast));

        auto create_default = [&](std::string name, auto &&ftor)
        {
            const auto fp = dot_proast/name;
            if (!std::filesystem::exists(fp))
            {
                std::ofstream fo{fp};
                ftor(fo);
            }
        };

        create_default(".gitignore", [](auto &fo){
                fo << "metadata.naft" << std::endl;
                fo << "trash/" << std::endl;
                });
        create_default("config.naft", [](auto &fo){
                });

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
