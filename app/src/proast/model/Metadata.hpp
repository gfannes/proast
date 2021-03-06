#ifndef HEADER_proast_model_Metadata_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Metadata_hpp_ALREADY_INCLUDED

#include <proast/model/enums.hpp>
#include <proast/types.hpp>
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
        std::optional<double>       effort;
        std::optional<std::string>  live;
        std::optional<std::string>  due;
        std::optional<std::string>  dead;
        std::optional<double>       completion_pct;
        std::optional<double>       volume_db;
        std::optional<double>       impact;
        Tags                        tags;
        std::optional<State>        state;
        bool                        done = false;
        bool                        order_sequential = false;

        double get_volume_db() const;
        double get_impact() const;
        double get_age() const;
        double get_completion_frac() const;
        double get_todo() const;

        bool has_local_data() const;

        void stream(gubg::naft::Node &);
        bool parse(gubg::naft::Range &);

        void set_when_unset(const Metadata &other);

    private:
    };
} } 

#endif
