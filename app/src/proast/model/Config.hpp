#ifndef HEADER_proast_model_Config_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Config_hpp_ALREADY_INCLUDED

#include <gubg/file/system.hpp>
#include <filesystem>
#include <string>

namespace proast { namespace model { 

    class Config
    {
    public:
        static std::filesystem::path directory(const std::filesystem::path &root);
        static std::filesystem::path filepath(const std::filesystem::path &root);
        static bool create_default(const std::filesystem::path &root);

        std::string extension() const {return extension_;}
        std::string index_filename() const {return index_name_+extension_;}

        bool reload(const std::filesystem::path &fp);

    private:
        std::string index_name_ = "readme";
        std::string extension_ = ".md";
    };

} } 

#endif
