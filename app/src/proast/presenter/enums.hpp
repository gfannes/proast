#ifndef HEADER_proast_presenter_enums_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_enums_hpp_ALREADY_INCLUDED

#include <string>
#include <ostream>
#include <optional>

namespace proast { namespace presenter { 
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
    ftor(type, View) \
    ftor(type, Edit) \
    ftor(type, Shell) \
    ftor(type, Run) \

    define_enum_(Open, my_each_value_);
#undef my_each_value_

#define my_each_value_(type, ftor) \
    ftor(type, BookmarkRegister) \
    ftor(type, BookmarkJump) \
    ftor(type, SetData) \
    ftor(type, SetMetadata) \
    ftor(type, SetState) \
    ftor(type, SetOrder) \
    ftor(type, ShowMetadata) \
    ftor(type, Create) \
    ftor(type, Delete) \
    ftor(type, Duplicate) \
    ftor(type, Paste) \
    ftor(type, Rename) \
    ftor(type, Export) \
    ftor(type, Run) \
    ftor(type, Search) \
    ftor(type, Plan) \

    define_enum_(State, my_each_value_);
#undef my_each_value_

#define my_each_value_(type, ftor) \
    ftor(type, Effort) \
    ftor(type, Volume) \
    ftor(type, Impact) \
    ftor(type, CompletionPct) \
    ftor(type, Live) \
    ftor(type, Dead) \
    ftor(type, Tag) \
    ftor(type, Dependency) \

    define_enum_(MetadataField, my_each_value_);
#undef my_each_value_

#define my_each_value_(type, ftor) \
    ftor(type, One) \
    ftor(type, Append) \
    ftor(type, Clear) \

    define_enum_(Delete, my_each_value_);
#undef my_each_value_

#undef define_enum_
#undef ftor_case_
#undef ftor_value_
} } 

#endif
