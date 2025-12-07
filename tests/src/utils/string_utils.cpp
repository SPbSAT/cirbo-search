#include "utils/string_utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>
#include <string_view>

using namespace cirbo;
using namespace cirbo::string_utils;

TEST_CASE("StringUtils isEmptyLine", "[string_utils]")
{
    REQUIRE(isEmptyLine(""));
    REQUIRE(isEmptyLine("\n"));
    REQUIRE(isEmptyLine("\r"));
    REQUIRE(isEmptyLine("\r\n"));

    REQUIRE_FALSE(isEmptyLine("1"));
    REQUIRE_FALSE(isEmptyLine("\t"));
}

TEST_CASE("StringUtils TrimNewlineCharacter", "[string_utils]")
{
    std::string const a = "abc\r";
    std::string const b = "\n";
    std::string const c = "123\n 456\r\n";

    std::string_view asv = a;
    std::string_view bsv = b;
    std::string_view csv = c;

    trimNewlineCharacter(&asv);
    trimNewlineCharacter(&bsv);
    trimNewlineCharacter(&csv);

    REQUIRE(asv == "abc");
    REQUIRE(bsv == "");
    REQUIRE(csv == "123\n 456");
}

TEST_CASE("StringUtils TrimSpaces", "[string_utils]")
{
    std::string const a = "    abc";
    std::string const b = "abc       ";
    std::string const c = "abc";
    std::string const d = "abc def ";
    std::string const e = "";
    std::string const f = "\t\t\tabc";
    std::string const g = "abc\t\t   \t \t  \t";
    std::string const h = "\t\t  \t \t";

    std::string_view asv = a;
    std::string_view bsv = b;
    std::string_view csv = c;
    std::string_view dsv = d;
    std::string_view esv = e;
    std::string_view fsv = f;
    std::string_view gsv = g;
    std::string_view hsv = h;

    trimSpaces(&asv);
    trimSpaces(&bsv);
    trimSpaces(&csv);
    trimSpaces(&dsv);
    trimSpaces(&esv);
    trimSpaces(&fsv);
    trimSpaces(&gsv);
    trimSpaces(&hsv);

    REQUIRE(asv == "abc");
    REQUIRE(bsv == "abc");
    REQUIRE(csv == "abc");
    REQUIRE(dsv == "abc def");
    REQUIRE(esv.empty());
    REQUIRE(fsv == "abc");
    REQUIRE(gsv == "abc");
    REQUIRE(hsv.empty());
}
