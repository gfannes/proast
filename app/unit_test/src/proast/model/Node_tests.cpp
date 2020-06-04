#include <proast/model/Node.hpp>
#include <catch.hpp>
#include <iostream>
using namespace proast;

TEST_CASE("model::Node read tests", "[ut][model][Node][read]")
{
    struct Scn
    {
        std::string markdown;
    };
    struct Exp
    {
        model::Item item;
    };

    Scn scn;
    Exp exp;

    SECTION("default") { } 
    SECTION("Only title")
    {
        scn.markdown = R"(# My title)";
        exp.item.set_title("My title");
    }
    SECTION("Full")
    {
        scn.markdown = R"(# My title

Top-level
Description

## Requirements

* Must have this and that
  And it
  Better
  Be
  Good

## Whatever section

This is lost

## Design

* Bla
  * Subbullet1
  * Subbullet2
* Bli
  * Subbullet1
  * Subbullet2
)";
        exp.item.set_title("My title");
    }

    model::Node node;
    REQUIRE(model::read_markdown(node, scn.markdown));
    std::cout << node.value << std::endl;
    for (const auto &child: node.childs.nodes)
        std::cout << "  " << child.value << std::endl;
    REQUIRE(node.value.key() == exp.item.key());
    REQUIRE(node.value.title() == exp.item.title());
}

TEST_CASE("model::Node write tests", "[ut][model][Node][write]")
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

    model::Node node;

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
