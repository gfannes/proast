#ifndef HEADER_proast_presenter_enums_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_enums_hpp_ALREADY_INCLUDED

#include <string>

namespace proast { namespace presenter { 
#define ftor_value_(t,v) v,
#define ftor_case_(t,v) case t::v: return #v;
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

#define my_each_value_(type, ftor) \
    ftor(type, View) \
    ftor(type, Edit) \
    ftor(type, Shell) \

    define_enum_(Open, my_each_value_);
#undef my_each_value_

#define my_each_value_(type, ftor) \
    ftor(type, BookmarkRegister) \
    ftor(type, BookmarkJump) \
    ftor(type, SetMetadataField) \
    ftor(type, ShowMetadataField) \
    ftor(type, Create) \
    ftor(type, Delete) \
    ftor(type, Paste) \
    ftor(type, Rename) \

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

#undef define_enum_
#undef ftor_case_
#undef ftor_value_
} } 

#endif
