#include <proast/model/Markdown.hpp>
#include <catch.hpp>
#include <iostream>
using namespace proast;

TEST_CASE("model::Node read from directory tests", "[ut][model][Node][read][directory]")
{
    const auto cwd = std::filesystem::current_path();

    model::Node node;
    model::Config config;
    REQUIRE(model::markdown::read_directory(node, cwd, config));
    REQUIRE(model::markdown::write_directory(cwd/"generated", node, config));
}

TEST_CASE("model::Node read tests", "[ut][model][Node][read][string]")
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
        exp.item.title = "My title";
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
        exp.item.title = "My title";
    }

    model::Node node;
    REQUIRE(model::markdown::read_string(node, scn.markdown));
    std::cout << node.value << std::endl;
    for (const auto &child: node.childs.nodes)
        std::cout << "  " << child.value << std::endl;
    REQUIRE(node.value.key == exp.item.key);
    REQUIRE(node.value.title == exp.item.title);
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
        node.value.key = "create_unit_test";
        exp.markdown += "# Create unit test\n";
    }
    SECTION("all sections")
    {
        node.value.key = "create_unit_test";
        node.value.description.push_back("Creating unit tests is the way to go.");

        //Requirements
        {
            auto &child = node.childs.append();
            child.value.set(model::Type::Requirement, 0);
            child.value.title = "Embedded requirement";
            child.value.description.push_back("Explain here what should be achieved");
        }
        {
            auto &child = node.childs.append();
            child.value.set(model::Type::Requirement, "what");
        }

        //Design
        {
            auto &child = node.childs.append();
            child.value.set(model::Type::Design, 0);
            child.value.title = "Embedded design";
            child.value.description.push_back("Make some decisions to narrow-down the solution space");
        }
        {
            auto &child = node.childs.append();
            child.value.set(model::Type::Design, "principle");
        }

        //Features
        {
            auto &child = node.childs.append();
            child.value.set(model::Type::Feature, 0);
            child.value.title = "Embedded feature";
            child.value.description.push_back("Breakdown this feature in client-valued subfeatures");
        }
        {
            auto &child = node.childs.append();
            child.value.set(model::Type::Feature, "feature");
        }
        {
            auto &child = node.childs.append();
            child.value.link = {"a", "b", "c"};
        }
    }

    std::string markdown;
    REQUIRE(model::markdown::write_string(markdown, node));
    std::cout << markdown;
    /* REQUIRE(markdown == exp.markdown); */
}
