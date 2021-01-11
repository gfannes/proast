#include <proast/Tree.hpp>
#include <catch.hpp>

TEST_CASE("Tree tests", "[ut][Tree]")
{
    proast::Tree tree;
    REQUIRE(tree.add(std::filesystem::current_path(), proast::Tree::Config{}));
}
