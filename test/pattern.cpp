#include <catch.hpp>
#include "pattern_parse.hpp"

using namespace Neptools;

// static test...
auto fuck_you_cpp_committee = NEPTOOLS_PATTERN("01 02 03 04");
static_assert(std::is_same<
    decltype(fuck_you_cpp_committee),
    PatternParse::Pattern<
        PatternParse::ByteSequence<1,2,3,4>,
        PatternParse::ByteSequence<0xff,0xff,0xff,0xff>>>::value,
    "simple pattern");

auto why_cant_I_use_fucking_lambdas = NEPTOOLS_PATTERN("11 05 f3 14");
static_assert(std::is_same<
    decltype(why_cant_I_use_fucking_lambdas),
    PatternParse::Pattern<
        PatternParse::ByteSequence<0x11,0x05,0xf3,0x14>,
        PatternParse::ByteSequence<0xff,0xff,0xff,0xff>>>::value,
    "multi digit");

auto in_an_unevaluated_context_like_decltype = NEPTOOLS_PATTERN("11 5 f3 4");
static_assert(std::is_same<
    decltype(in_an_unevaluated_context_like_decltype),
    PatternParse::Pattern<
        PatternParse::ByteSequence<0x11,0x05,0xf3,0x04>,
        PatternParse::ByteSequence<0xff,0xff,0xff,0xff>>>::value,
    "shortcuts");

auto or_why_cant_you_add_a_sane_way_to_use = NEPTOOLS_PATTERN("11 ?? f3 04");
static_assert(std::is_same<
    decltype(or_why_cant_you_add_a_sane_way_to_use),
    PatternParse::Pattern<
        PatternParse::ByteSequence<0x11,0x00,0xf3,0x04>,
        PatternParse::ByteSequence<0xff,0x00,0xff,0xff>>>::value,
    "placeholder");

auto fucking_strings_as_template_arguments = NEPTOOLS_PATTERN("11 ? f3 04");
static_assert(std::is_same<
    decltype(fucking_strings_as_template_arguments),
    PatternParse::Pattern<
        PatternParse::ByteSequence<0x11,0x00,0xf3,0x04>,
        PatternParse::ByteSequence<0xff,0x00,0xff,0xff>>>::value,
    "short placeholder");
// PS. you should add ctr instead of shit like a wrapper around cairo

Byte data[] = {
    /* 00 */ 0xff, 0xf0, 0x64, 0x22, 0x50, 0xca, 0x9f, 0x23,
    /* 08 */ 0x92, 0xf7, 0xb3, 0x8f, 0xb1, 0x30, 0x8a, 0xd6,
    /* 10 */ 0x1e, 0x38, 0xd8, 0xf3, 0xa7, 0xfa, 0x98, 0xee,
    /* 18 */ 0x58, 0xec, 0x21, 0xa8, 0xfe, 0x00, 0x1c, 0xfb,
    /* 20 */ 0x5e, 0x0a, 0x48, 0x38, 0xc5, 0x64, 0xb5, 0xbe,
    /* 28 */ 0xc1, 0x48, 0xdd, 0x7f, 0x3e, 0x58, 0xec, 0x21,
    /* 30 */ 0x82, 0xf4, 0xca, 0x11, 0x47, 0x22, 0xcf, 0x98,
    /* 38 */ 0x40, 0xfe, 0x18, 0x0e, 0x2e, 0xb9, 0xfc, 0xce,
};
TEST_CASE("simple patterns", "[pattern]")
{
    Pattern p = NEPTOOLS_PATTERN("a8 fe 0 1c"); //middle
    CHECK(p.MaybeFind({data, sizeof(data)}) == data + 0x1b);
    CHECK(p.Find({data, sizeof(data)}) == data + 0x1b);

    p = NEPTOOLS_PATTERN("ff f0 64"); //beginning
    CHECK(p.MaybeFind({data, sizeof(data)}) == data);
    CHECK(p.Find({data, sizeof(data)}) == data);

    p = NEPTOOLS_PATTERN("18 e 2e b9 fc ce"); //end
    CHECK(p.MaybeFind({data, sizeof(data)}) == data + 0x3a);
    CHECK(p.Find({data, sizeof(data)}) == data + 0x3a);
}
TEST_CASE("simple not match", "[pattern]")
{
    auto p = NEPTOOLS_PATTERN("12 34 56 76");
    CHECK(p.MaybeFind({data, sizeof(data)}) == nullptr);
    CHECK_THROWS_AS(p.Find({data, sizeof(data)}), std::runtime_error);
}
TEST_CASE("multiple match", "[pattern]")
{
    auto p = NEPTOOLS_PATTERN("58 ec 21");
    CHECK(p.MaybeFind({data, sizeof(data)}) == nullptr);
    CHECK_THROWS_AS(p.Find({data, sizeof(data)}), std::runtime_error);
}

TEST_CASE("wildcards", "[pattern]")
{
    Pattern p = NEPTOOLS_PATTERN("48 ? c5 ? ? be"); // mid
    CHECK(p.MaybeFind({data, sizeof(data)}) == data + 0x22);
    CHECK(p.Find({data, sizeof(data)}) == data + 0x22);

    p = NEPTOOLS_PATTERN("ff ? ? 22 50"); // begin
    CHECK(p.MaybeFind({data, sizeof(data)}) == data);
    CHECK(p.Find({data, sizeof(data)}) == data);


    p = NEPTOOLS_PATTERN("e ? b9 ? ce"); // end
    CHECK(p.MaybeFind({data, sizeof(data)}) == data + 0x3b);
    CHECK(p.Find({data, sizeof(data)}) == data + 0x3b);

    p = NEPTOOLS_PATTERN("? ? fe 00 ?"); // mid
    CHECK(p.MaybeFind({data, sizeof(data)}) == data + 0x1a);
    CHECK(p.Find({data, sizeof(data)}) == data + 0x1a);

    p = NEPTOOLS_PATTERN("? ? 64 22 ?"); // begin
    CHECK(p.MaybeFind({data, sizeof(data)}) == data);
    CHECK(p.Find({data, sizeof(data)}) == data);

    p = NEPTOOLS_PATTERN("? 2e b9 ? ?"); // end
    CHECK(p.MaybeFind({data, sizeof(data)}) == data + 0x3b);
    CHECK(p.Find({data, sizeof(data)}) == data + 0x3b);
}
TEST_CASE("wildcards not match", "[pattern]")
{
    Pattern p = NEPTOOLS_PATTERN("12 34 ?? 56 ?? 78");
    CHECK(p.MaybeFind({data, sizeof(data)}) == nullptr);
    CHECK_THROWS_AS(p.Find({data, sizeof(data)}), std::runtime_error);

    p = NEPTOOLS_PATTERN("? ? 12 34 56");
    CHECK(p.MaybeFind({data, sizeof(data)}) == nullptr);
    CHECK_THROWS_AS(p.Find({data, sizeof(data)}), std::runtime_error);

    p = NEPTOOLS_PATTERN("12 34 ? 56 ? ? ?");
    CHECK(p.MaybeFind({data, sizeof(data)}) == nullptr);
    CHECK_THROWS_AS(p.Find({data, sizeof(data)}), std::runtime_error);
}
TEST_CASE("wildcards multiple match", "[pattern]")
{
    Pattern p = NEPTOOLS_PATTERN("58 ? 21");
    CHECK(p.MaybeFind({data, sizeof(data)}) == nullptr);
    CHECK_THROWS_AS(p.Find({data, sizeof(data)}), std::runtime_error);

    p = NEPTOOLS_PATTERN("? 58 ? ?");
    CHECK(p.MaybeFind({data, sizeof(data)}) == nullptr);
    CHECK_THROWS_AS(p.Find({data, sizeof(data)}), std::runtime_error);

    p = NEPTOOLS_PATTERN("? ? ? ?");
    CHECK(p.MaybeFind({data, sizeof(data)}) == nullptr);
    CHECK_THROWS_AS(p.Find({data, sizeof(data)}), std::runtime_error);
}

TEST_CASE("mask", "[pattern]")
{
    Byte pat[]  = { 0x58, 0xe0, 0x21, 0x28, 0x3e };
    Byte mask[] = { 0xff, 0xf0, 0xff, 0x2f, 0x3f };
    auto p = Pattern{pat, mask, 5};
    CHECK(p.MaybeFind({data, sizeof(data)}) == data + 0x18);
    CHECK(p.Find({data, sizeof(data)}) == data + 0x18);
}

TEST_CASE("mask no match", "[pattern]")
{
    Byte pat[]  = { 0x58, 0xe0, 0x21, 0x18, 0x3e };
    Byte mask[] = { 0xff, 0xf0, 0xff, 0x1f, 0x3f };
    auto p = Pattern{pat, mask, 5};
    CHECK(p.MaybeFind({data, sizeof(data)}) == nullptr);
    CHECK_THROWS_AS(p.Find({data, sizeof(data)}), std::runtime_error);
}

TEST_CASE("mask no 0xff", "[pattern]")
{
    Byte pat[]  = { 0x58, 0xe0, 0x20, 0x28, 0x3e };
    Byte mask[] = { 0x5f, 0xf0, 0xf2, 0x2f, 0x3f };
    auto p = Pattern{pat, mask, 5};
    CHECK(p.MaybeFind({data, sizeof(data)}) == data + 0x18);
    CHECK(p.Find({data, sizeof(data)}) == data + 0x18);

    Byte pat2[]  = { 0x58, 0xe0, 0x20, 0x18, 0x3e };
    Byte mask2[] = { 0x5f, 0xf0, 0xf2, 0x1f, 0x3f };
    p = Pattern{pat2, mask2, 5};
    CHECK(p.MaybeFind({data, sizeof(data)}) == nullptr);
    CHECK_THROWS_AS(p.Find({data, sizeof(data)}), std::runtime_error);
}
