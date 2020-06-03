#include <proast/model/Item.hpp>
#include <catch.hpp>
using namespace proast;

TEST_CASE("model::Item tests", "[ut][model][Item]")
{
    model::Item item;
    SECTION("fresh")
    {
        REQUIRE(item.type() == model::Type::Feature);
        REQUIRE(item.is_embedded() == true);
        REQUIRE(item.stem() == "");
    }
    SECTION("title")
    {
        item.set_key("create_unit_test");
        REQUIRE(item.title() == "Create unit test");
    }
}
