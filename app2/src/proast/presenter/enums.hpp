#ifndef HEADER_proast_presenter_enums_hpp_ALREADY_INCLUDED
#define HEADER_proast_presenter_enums_hpp_ALREADY_INCLUDED

#include <proast/unicode.hpp>
#include <string>

namespace proast { namespace presenter { 
    enum class Open
    {
        View, Edit, Shell,
    };

    enum class State
    {
        Idle, BookmarkRegister, BookmarkJump, SetMetadataField,
    };


    //Begin MetadataField values
#define unroll_proast_present_MetadataField_values(ftor) \
    ftor(Effort) \
    ftor(Volume) \
    ftor(Impact) \
    ftor(CompletionPct) \
    ftor(Live) \
    ftor(Dead) \

    enum class MetadataField
    {
#define ftor(v) v,
        unroll_proast_present_MetadataField_values(ftor)
#undef ftor
    };
    inline std::wstring to_wstring(MetadataField v)
    {
        switch (v)
        {
#define ftor(v) case MetadataField::v: return WIDEN(#v);
            unroll_proast_present_MetadataField_values(ftor)
#undef ftor
            default: break;
        }
        return L"<Unknown MetadataField>";
    }
    //End MetadataField values
} } 

#endif
