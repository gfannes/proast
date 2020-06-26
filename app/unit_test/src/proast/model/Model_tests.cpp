#include <proast/model/Model.hpp>
#include <gubg/file/system.hpp>
#include <catch.hpp>
#include <iostream>
using namespace proast;

TEST_CASE("model::Model rename tests", "[ut][model][Model]")
{
    const auto dir = std::filesystem::current_path()/"app"/"unit_test"/"tmp"/"rename";
    std::filesystem::create_directories(dir);
    std::cout << dir << std::endl;
    gubg::file::write(R"(## Features

* [external](key:a)
* [external](key:b)
)", dir/"readme.md");

    auto a = dir/"a";
    std::filesystem::create_directories(a);
    gubg::file::write("", a/"readme.md");

    auto b = dir/"b";
    std::filesystem::create_directories(b);
    gubg::file::write(R"(## Features

* [external](path:/rename/a))", b/"readme.md");

    model::Model m{{dir}};
    {
        const auto ok = m();
        std::cout << log::content() << std::endl;
        REQUIRE(ok);
    }
    std::cout << m << std::endl;

    m.set_path({"rename","a"});
    std::cout << m << std::endl;

    {
        const auto ok = m.rename_item("c");
        std::cout << log::content() << std::endl;
        REQUIRE(ok);
    }
    std::cout << m << std::endl;
}
