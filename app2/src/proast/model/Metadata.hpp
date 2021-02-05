#ifndef HEADER_proast_model_Metadata_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Metadata_hpp_ALREADY_INCLUDED

#include <proast/model/Dependencies.hpp>
#include <gubg/naft/Node.hpp>
#include <gubg/naft/Range.hpp>
#include <optional>
#include <set>
#include <string>
#include <ostream>

namespace proast { namespace model { 
    class Metadata
    {
    public:
        using Tag = std::wstring;
        using Tags = std::set<Tag>;

        std::optional<double>       my_effort;
        std::optional<std::wstring> my_live;
        std::optional<std::wstring> my_dead;
        std::optional<double>       my_completion_pct;
        std::optional<double>       my_volume_db;
        std::optional<double>       my_impact;
        Tags                        my_tags;

        bool has_local_data() const;

        Dependencies dependencies;

        double effort() const;
        Tags tags() const;

        void stream(gubg::naft::Node &);
        bool parse(gubg::naft::Range &);

        void set_when_unset(const Metadata &other);

    private:
    };
} } 

#endif
