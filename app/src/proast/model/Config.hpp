#ifndef HEADER_proast_model_Config_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Config_hpp_ALREADY_INCLUDED

#include <gubg/file/system.hpp>
#include <filesystem>
#include <string>

namespace proast { namespace model { 

    class Config
    {
    public:
        static std::filesystem::path filepath(const std::filesystem::path &root);
        static bool create_default(const std::filesystem::path &root);

        std::string extension() const {return extension_;}
        std::string index_name() const {return index_name_;}
        std::string index_filename() const {return index_name_+extension_;}

        std::string cost_unit() const {return cost_unit_;}

        std::filesystem::path content_fp_leaf(const std::filesystem::path &directory) const;
        std::filesystem::path content_fp_nonleaf(const std::filesystem::path &directory) const;

        bool reload(const std::filesystem::path &fp);

    private:
        std::string index_name_ = "readme";
        std::string extension_ = ".md";
        std::string cost_unit_ = "days";
    };

} } 

#endif
