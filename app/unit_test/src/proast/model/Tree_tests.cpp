#include <proast/model/Tree.hpp>
#include <gubg/tree/stream.hpp>
#include <catch.hpp>
#include <iostream>
using namespace proast;

TEST_CASE("model::Tree tests", "[ut][model][Tree]")
{
    const auto cwd = std::filesystem::current_path();

    SECTION("find_root")
    {
        std::filesystem::path root;
        REQUIRE(model::Tree::find_root_filepath(root, cwd/"a/b/c"));
        REQUIRE(root == cwd);
    }

    SECTION("folder")
    {
        model::Tree tree;
        REQUIRE(tree.load(cwd));
        std::cout << tree << std::endl;
    }
    SECTION("file")
    {
        model::Tree tree;
        REQUIRE(tree.load(cwd/"readme.md"));
        std::cout << tree << std::endl;
    }
}
