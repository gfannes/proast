#include <proast/model/Node.hpp>
#include <catch.hpp>
#include <iostream>
using namespace proast;

TEST_CASE("model::Node tests", "[ut][model][Node]")
{
    struct Scn
    {
    };
    struct Exp
    {
        std::string markdown = R"(<!--
[proast](status:todo)
-->
)";
    };

    Scn scn;
    Exp exp;

    model::Node_ node;

    SECTION("default") { }
    SECTION("title")
    {
        node.value.set_key("create_unit_test");
        exp.markdown += "# Create unit test\n";
    }
    SECTION("all sections")
    {
        node.value.set_key("create_unit_test");
        node.value.set_description("Creating unit tests is the way to go.");

        //Requirements
        {
            auto &child = node.childs.append();
            child.value.set_key(model::Type::Requirement, 0);
            child.value.set_title("Embedded requirement");
            child.value.set_description("Explain here what should be achieved");
        }
        {
            auto &child = node.childs.append();
            child.value.set_key(model::Type::Requirement, "what");
        }

        //Design
        {
            auto &child = node.childs.append();
            child.value.set_key(model::Type::Design, 0);
            child.value.set_title("Embedded design");
            child.value.set_description("Make some decisions to narrow-down the solution space");
        }
        {
            auto &child = node.childs.append();
            child.value.set_key(model::Type::Design, "principle");
        }

        //Features
        {
            auto &child = node.childs.append();
            child.value.set_key(model::Type::Feature, 0);
            child.value.set_title("Embedded feature");
            child.value.set_description("Breakdown this feature in client-valued subfeatures");
        }
        {
            auto &child = node.childs.append();
            child.value.set_key(model::Type::Feature, "feature");
        }
        {
            auto &child = node.childs.append();
            child.value.link = {"a", "b", "c"};
        }
    }

    std::string markdown;
    REQUIRE(write_markdown(markdown, node));
    std::cout << markdown;
    /* REQUIRE(markdown == exp.markdown); */
}

