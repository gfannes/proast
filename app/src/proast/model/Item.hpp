#ifndef HEADER_proast_model_Item_hpp_ALREADY_INCLUDED
#define HEADER_proast_model_Item_hpp_ALREADY_INCLUDED

#include <proast/model/Path.hpp>
#include <gubg/markup/Document.hpp>
#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace proast { namespace model { 

    //TODO: Replace this with a real struct
    //For now, std::string was chosen because this can be compared when using the format YYYYMMDD
    using DateTime = std::string;

    enum class State
    {
        Unclear, Clear, Thinking, Designed, Implementing, Done,
    };
    std::string hr(State);
    inline std::string hr(std::optional<State> state) {if (state) return hr(*state); return "nullopt";}
    std::optional<State> to_status(const std::string &str);

    enum class Priority
    {
        Must, Should, Could, Wont,
    };
    std::string hr(Priority);
    std::optional<Priority> to_priority(const std::string &str);

    enum class Type
    {
        Requirement, Design, Feature, Free, File, Directory, 
    };
    std::string hr(Type);
    inline std::string hr(std::optional<Type> type) {if (type) return hr(*type); return "nullopt";}

    enum class Style
    {
        Title, Section, Bullet, Margin,
    };
    std::string hr(Style);

    std::string key_as_title(const std::string &str);
    std::string title_as_key(const std::string &str);

    class Item
    {
    public:
        std::optional<Type> type;
        std::string key;
        void set(Type t, const std::string &k) {type = t; key = k;}

        std::string title;

        std::optional<DateTime> deadline;
        std::optional<double> my_cost;
        double total_cost = 0;
        double done_cost = 0;
        std::optional<Style> style;
        std::optional<State> state;
        std::optional<Path> link;
        std::optional<Priority> priority;

        std::optional<std::filesystem::path> content_fp;

        std::vector<std::string> description;

        //Indicates if this item is embedded in the content of its parent
        bool is_embedded() const;

        void clear() {*this = Item{};}

        //Members to remove

        //Replace this with markdown::ast::Tree
        gubg::markup::Document preview;
        std::string active_child_key;
        std::optional<std::filesystem::path> directory;

        void stream(std::ostream &os) const;

    private:
        std::string key_;
        std::string title_;
    };

    inline std::ostream &operator<<(std::ostream &os, const Item &item)
    {
        item.stream(os);
        return os;
    }

} } 

#endif
