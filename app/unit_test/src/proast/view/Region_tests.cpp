#include <proast/view/Region.hpp>
#include <catch.hpp>
#include <iostream>
using namespace proast;

TEST_CASE("view::Region tests", "[ut][view][Region]")
{
    view::Region region0;

    SECTION("default ctor")
    {
        REQUIRE(region0.x_begin() == 0);
        REQUIRE(region0.y_begin() == 0);
        REQUIRE(region0.x_end() == 0);
        REQUIRE(region0.y_end() == 0);
        REQUIRE(region0.x_size() == 0);
        REQUIRE(region0.y_size() == 0);
    }

    region0 = view::Region(100,200 ,100,200);
    SECTION("pop_left")
    {
        auto region1 = region0.pop_left(10);
        REQUIRE(region1 == view::Region(100,200, 100,110));
        REQUIRE(region0 == view::Region(100,200, 110,200));
    }
    SECTION("pop_right")
    {
        auto region1 = region0.pop_right(10);
        REQUIRE(region1 == view::Region(100,200, 190,200));
        REQUIRE(region0 == view::Region(100,200, 100,190));
    }
    SECTION("pop_top")
    {
        auto region1 = region0.pop_top(10);
        REQUIRE(region1 == view::Region(100,110, 100,200));
        REQUIRE(region0 == view::Region(110,200, 100,200));
    }
    SECTION("pop_bottom")
    {
        auto region1 = region0.pop_bottom(10);
        REQUIRE(region1 == view::Region(190,200, 100,200));
        REQUIRE(region0 == view::Region(100,190, 100,200));
    }
}
