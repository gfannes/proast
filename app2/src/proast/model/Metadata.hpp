#ifndef HEADER_proast_model_Metadata_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Metadata_hpp_ALREADY_INCLUDED

#include <optional>
#include <set>
#include <string>

namespace proast { namespace model { 
    class Metadata
    {
    public:
        using Tag = std::wstring;
        using Tags = std::set<Tag>;

        std::optional<double>       my_effort;
        std::optional<Tags>         my_tags = Tags{L"abc", L"aaa"};
        std::optional<std::wstring> my_live;
        std::optional<std::wstring> my_dead;
        std::optional<double>       my_completion_pct;
        std::optional<double>       my_volume_db;
        std::optional<double>       my_impact;

        double effort = 0.0;
        Tags tags;

        void reset_aggregated_data();

    private:
    };
} } 

#endif
