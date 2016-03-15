#include <catch.hpp>
#include "pattern_parse.hpp"

using namespace Neptools;

// static test...
static_assert(std::is_same<
    decltype("01 02 03 04"_pattern),
    PatternParse::Pattern<
        PatternParse::ByteSequence<1,2,3,4>,
        PatternParse::ByteSequence<0xff,0xff,0xff,0xff>>>::value,
    "simple pattern");

static_assert(std::is_same<
    decltype("11 05 f3 14"_pattern),
    PatternParse::Pattern<
        PatternParse::ByteSequence<0x11,0x05,0xf3,0x14>,
        PatternParse::ByteSequence<0xff,0xff,0xff,0xff>>>::value,
    "multi digit");

static_assert(std::is_same<
    decltype("11 5 f3 4"_pattern),
    PatternParse::Pattern<
        PatternParse::ByteSequence<0x11,0x05,0xf3,0x04>,
        PatternParse::ByteSequence<0xff,0xff,0xff,0xff>>>::value,
    "shortcuts");

static_assert(std::is_same<
    decltype("11 ?? f3 04"_pattern),
    PatternParse::Pattern<
        PatternParse::ByteSequence<0x11,0x00,0xf3,0x04>,
        PatternParse::ByteSequence<0xff,0x00,0xff,0xff>>>::value,
    "placeholder");

static_assert(std::is_same<
    decltype("11 ? f3 04"_pattern),
    PatternParse::Pattern<
        PatternParse::ByteSequence<0x11,0x00,0xf3,0x04>,
        PatternParse::ByteSequence<0xff,0x00,0xff,0xff>>>::value,
    "short placeholder");

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
    Pattern p = "a8 fe 0 1c"_pattern; //middle
    CHECK(p.MaybeFind(data, sizeof(data)) == data + 0x1b);
    CHECK(p.Find(data, sizeof(data)) == data + 0x1b);

    p = "ff f0 64"_pattern; //beginning
    CHECK(p.MaybeFind(data, sizeof(data)) == data);
    CHECK(p.Find(data, sizeof(data)) == data);

    p = "18 e 2e b9 fc ce"_pattern; //end
    CHECK(p.MaybeFind(data, sizeof(data)) == data + 0x3a);
    CHECK(p.Find(data, sizeof(data)) == data + 0x3a);
}
TEST_CASE("simple not match", "[pattern]")
{
    auto p = "12 34 56 76"_pattern;
    CHECK(p.MaybeFind(data, sizeof(data)) == nullptr);
    CHECK_THROWS_AS(p.Find(data, sizeof(data)), std::runtime_error);
}
TEST_CASE("multiple match", "[pattern]")
{
    auto p = "58 ec 21"_pattern;
    CHECK(p.MaybeFind(data, sizeof(data)) == nullptr);
    CHECK_THROWS_AS(p.Find(data, sizeof(data)), std::runtime_error);
}

TEST_CASE("wildcards", "[pattern]")
{
    Pattern p = "48 ? c5 ? ? be"_pattern; // mid
    CHECK(p.MaybeFind(data, sizeof(data)) == data + 0x22);
    CHECK(p.Find(data, sizeof(data)) == data + 0x22);

    p = "ff ? ? 22 50"_pattern; // begin
    CHECK(p.MaybeFind(data, sizeof(data)) == data);
    CHECK(p.Find(data, sizeof(data)) == data);


    p = "e ? b9 ? ce"_pattern; // end
    CHECK(p.MaybeFind(data, sizeof(data)) == data + 0x3b);
    CHECK(p.Find(data, sizeof(data)) == data + 0x3b);

    p = "? ? fe 00 ?"_pattern; // mid
    CHECK(p.MaybeFind(data, sizeof(data)) == data + 0x1a);
    CHECK(p.Find(data, sizeof(data)) == data + 0x1a);

    p = "? ? 64 22 ?"_pattern; // begin
    CHECK(p.MaybeFind(data, sizeof(data)) == data);
    CHECK(p.Find(data, sizeof(data)) == data);

    p = "? 2e b9 ? ?"_pattern; // end
    CHECK(p.MaybeFind(data, sizeof(data)) == data + 0x3b);
    CHECK(p.Find(data, sizeof(data)) == data + 0x3b);
}
TEST_CASE("wildcards not match", "[pattern]")
{
    Pattern p = "12 34 ?? 56 ?? 78"_pattern;
    CHECK(p.MaybeFind(data, sizeof(data)) == nullptr);
    CHECK_THROWS_AS(p.Find(data, sizeof(data)), std::runtime_error);

    p = "? ? 12 34 56"_pattern;
    CHECK(p.MaybeFind(data, sizeof(data)) == nullptr);
    CHECK_THROWS_AS(p.Find(data, sizeof(data)), std::runtime_error);

    p = "12 34 ? 56 ? ? ?"_pattern;
    CHECK(p.MaybeFind(data, sizeof(data)) == nullptr);
    CHECK_THROWS_AS(p.Find(data, sizeof(data)), std::runtime_error);
}
TEST_CASE("wildcards multiple match", "[pattern]")
{
    Pattern p = "58 ? 21"_pattern;
    CHECK(p.MaybeFind(data, sizeof(data)) == nullptr);
    CHECK_THROWS_AS(p.Find(data, sizeof(data)), std::runtime_error);

    p = "? 58 ? ?"_pattern;
    CHECK(p.MaybeFind(data, sizeof(data)) == nullptr);
    CHECK_THROWS_AS(p.Find(data, sizeof(data)), std::runtime_error);

    p = "? ? ? ?"_pattern;
    CHECK(p.MaybeFind(data, sizeof(data)) == nullptr);
    CHECK_THROWS_AS(p.Find(data, sizeof(data)), std::runtime_error);
}

TEST_CASE("mask", "[pattern]")
{
    Byte pat[]  = { 0x58, 0xe0, 0x21, 0x28, 0x3e };
    Byte mask[] = { 0xff, 0xf0, 0xff, 0x2f, 0x3f };
    auto p = Pattern{pat, mask, 5};
    CHECK(p.MaybeFind(data, sizeof(data)) == data + 0x18);
    CHECK(p.Find(data, sizeof(data)) == data + 0x18);
}

TEST_CASE("mask no match", "[pattern]")
{
    Byte pat[]  = { 0x58, 0xe0, 0x21, 0x18, 0x3e };
    Byte mask[] = { 0xff, 0xf0, 0xff, 0x1f, 0x3f };
    auto p = Pattern{pat, mask, 5};
    CHECK(p.MaybeFind(data, sizeof(data)) == nullptr);
    CHECK_THROWS_AS(p.Find(data, sizeof(data)), std::runtime_error);
}

TEST_CASE("mask no 0xff", "[pattern]")
{
    Byte pat[]  = { 0x58, 0xe0, 0x20, 0x28, 0x3e };
    Byte mask[] = { 0x5f, 0xf0, 0xf2, 0x2f, 0x3f };
    auto p = Pattern{pat, mask, 5};
    CHECK(p.MaybeFind(data, sizeof(data)) == data + 0x18);
    CHECK(p.Find(data, sizeof(data)) == data + 0x18);

    Byte pat2[]  = { 0x58, 0xe0, 0x20, 0x18, 0x3e };
    Byte mask2[] = { 0x5f, 0xf0, 0xf2, 0x1f, 0x3f };
    p = Pattern{pat2, mask2, 5};
    CHECK(p.MaybeFind(data, sizeof(data)) == nullptr);
    CHECK_THROWS_AS(p.Find(data, sizeof(data)), std::runtime_error);
}
