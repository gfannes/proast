#ifndef HEADER_proast_model_Metadata_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Metadata_hpp_ALREADY_INCLUDED

#include <filesystem>

namespace proast { namespace model { 
    class Metadata
    {
    public:
        void clear();

        bool load(const std::filesystem::path &);
        bool save(const std::filesystem::path &) const;
    private:
    };
} } 

#endif
