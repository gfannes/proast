#ifndef HEADER_proast_model_enums_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_enums_hpp_ALREADY_INCLUDED

#include <string>
#include <ostream>
#include <optional>

namespace proast { namespace model { 
#define ftor_value_(t,v) v,
#define ftor_case_(t,v) case t::v: return #v;
#define ftor_elif_(t,v) else if (str == #v) {return t::v;}
#define define_enum_(type, each_value) \
    enum class type \
    { \
        each_value(type, ftor_value_) \
    }; \
    inline std::string to_string(type v) \
    { \
        switch (v) \
        { \
            each_value(type, ftor_case_) \
            default: break; \
        } \
        return "<unknown value>"; \
    } \
    inline std::optional<type> to_##type(const std::string &str) \
    { \
        if (false) {} \
        each_value(type, ftor_elif_) \
        return std::nullopt; \
    } \
    inline std::ostream &operator<<(std::ostream &os, type v) \
    { \
        return os << to_string(v); \
    } \

#define my_each_value_(type, ftor) \
    ftor(type, Collecting) \
    ftor(type, Designing) \
    ftor(type, Starting) \
    ftor(type, Implementing) \
    ftor(type, Validating) \

    define_enum_(State, my_each_value_);
#undef my_each_value_

#define my_each_value_(type, ftor) \
    ftor(type, File) \
    ftor(type, Directory) \
    ftor(type, Virtual) \
    ftor(type, Link) \

    define_enum_(Type, my_each_value_);
#undef my_each_value_

#undef define_enum_
#undef ftor_case_
#undef ftor_value_
} } 

#endif
