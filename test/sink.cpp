#include "sink.hpp"
#include <catch.hpp>
#include <fstream>

#define MAYBE GENERATE(Catch::Generators::between(false, true))
TEST_CASE("small simple write", "[Sink]")
{
    char buf[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    auto sink = Sink::ToFile("tmp", 16, MAYBE);
    REQUIRE(sink->Tell() == 0);
    sink->Write(buf);
    REQUIRE(sink->Tell() == 16);
    sink.reset();

    char buf2[16];
    std::ifstream is{"tmp", std::ios_base::binary};
    is.read(buf2, 16);
    REQUIRE(is.good());
    REQUIRE(memcmp(buf, buf2, 16) == 0);

    is.get();
    REQUIRE(is.eof());
}

TEST_CASE("many small writes", "[Sink]")
{
    int buf[6] = {0,77,-123,98,77,-1};
    STATIC_ASSERT(sizeof(buf) == 24);

    static constexpr FilePosition SIZE = 2*1024*1024 / 24 * 24;
    auto sink = Sink::ToFile("tmp", SIZE, MAYBE);
    for (FilePosition i = 0; i < SIZE; i += 24)
    {
        buf[0] = i/24;
        sink->Write(buf);
    }
    REQUIRE(sink->Tell() == SIZE);
    sink.reset();

    std::unique_ptr<char[]> buf_exp{new char[SIZE]};
    for (FilePosition i = 0; i < SIZE; i += 24)
    {
        buf[0] = i/24;
        memcpy(buf_exp.get()+i, buf, 24);
    }

    std::unique_ptr<char[]> buf_act{new char[SIZE]};
    std::ifstream is{"tmp", std::ios_base::binary};
    is.read(buf_act.get(), SIZE);
    REQUIRE(is.good());
    REQUIRE(memcmp(buf_exp.get(), buf_act.get(), SIZE) == 0);

    is.get();
    REQUIRE(is.eof());
}

TEST_CASE("big write", "[Sink]")
{
    static constexpr FilePosition SIZE = 2*1024*1024;
    std::unique_ptr<Byte[]> buf{new Byte[SIZE]};
    for (size_t i = 0; i < SIZE; ++i)
        buf[i] = i;

    auto sink = Sink::ToFile("tmp", SIZE, MAYBE);
    REQUIRE(sink->Tell() == 0);
    sink->Write(buf.get(), SIZE);
    REQUIRE(sink->Tell() == SIZE);
    sink.reset();

    std::unique_ptr<char[]> buf2{new char[SIZE]};
    std::ifstream is{"tmp", std::ios_base::binary};
    is.read(buf2.get(), SIZE);
    REQUIRE(is.good());
    REQUIRE(memcmp(buf.get(), buf2.get(), SIZE) == 0);

    is.get();
    REQUIRE(is.eof());
}

TEST_CASE("small pad", "[Sink]")
{
    char buf[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    auto sink = Sink::ToFile("tmp", 16*3, MAYBE);
    REQUIRE(sink->Tell() == 0);
    sink->Write(buf);
    REQUIRE(sink->Tell() == 16);
    sink->Pad(16);
    REQUIRE(sink->Tell() == 2*16);
    sink->Write(buf);
    REQUIRE(sink->Tell() == 3*16);
    sink.reset();

    char buf2[16];
    std::ifstream is{"tmp", std::ios_base::binary};
    is.read(buf2, 16);
    REQUIRE(is.good());
    REQUIRE(memcmp(buf, buf2, 16) == 0);

    is.read(buf2, 16);
    REQUIRE(is.good());
    REQUIRE(std::count(buf2, buf2+16, 0) == 16);

    is.read(buf2, 16);
    REQUIRE(is.good());
    REQUIRE(memcmp(buf, buf2, 16) == 0);

    is.get();
    REQUIRE(is.eof());
}

TEST_CASE("many small pad", "[Sink]")
{
    char buf[10] = {4,5,6,7,8,9,10,11,12,13};
    static constexpr FilePosition SIZE = 2*1024*1024 / 24 * 24;
    auto sink = Sink::ToFile("tmp", SIZE, MAYBE);

    for (FilePosition i = 0; i < SIZE; i += 24)
    {
        sink->Write(buf);
        sink->Pad(14);
    }
    REQUIRE(sink->Tell() == SIZE);
    sink.reset();

    std::unique_ptr<char[]> buf_exp{new char[SIZE]};
    for (FilePosition i = 0; i < SIZE; i += 24)
    {
        memcpy(buf_exp.get()+i, buf, 10);
        memset(buf_exp.get()+i+10, 0, 14);
    }

    std::unique_ptr<char[]> buf_act{new char[SIZE]};
    std::ifstream is{"tmp", std::ios_base::binary};
    is.read(buf_act.get(), SIZE);
    REQUIRE(is.good());
    REQUIRE(memcmp(buf_exp.get(), buf_act.get(), SIZE) == 0);

    is.get();
    REQUIRE(is.eof());
}

TEST_CASE("large pad", "[Sink]")
{
    char buf[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    static constexpr FilePosition ZERO_SIZE = 2*1024*1024;
    auto sink = Sink::ToFile("tmp", 16*2 + ZERO_SIZE, MAYBE);
    REQUIRE(sink->Tell() == 0);
    sink->Write(buf);
    REQUIRE(sink->Tell() == 16);
    sink->Pad(ZERO_SIZE);
    REQUIRE(sink->Tell() == 16+ZERO_SIZE);
    sink->Write(buf);
    REQUIRE(sink->Tell() == 2*16+ZERO_SIZE);
    sink.reset();

    std::unique_ptr<char[]> buf2{new char[ZERO_SIZE]};
    std::ifstream is{"tmp", std::ios_base::binary};
    is.read(buf2.get(), 16);
    REQUIRE(is.good());
    REQUIRE(memcmp(buf, buf2.get(), 16) == 0);

    is.read(buf2.get(), ZERO_SIZE);
    REQUIRE(is.good());
    REQUIRE(std::count(buf2.get(), buf2.get()+ZERO_SIZE, 0) == ZERO_SIZE);

    is.read(buf2.get(), 16);
    REQUIRE(is.good());
    REQUIRE(memcmp(buf, buf2.get(), 16) == 0);

    is.get();
    REQUIRE(is.eof());
}

TEST_CASE("sink helpers", "[Sink]")
{
    auto sink = Sink::ToFile("tmp", 15, MAYBE);
    sink->WriteLittleUint8(247);
    sink->WriteLittleUint16(1234);
    sink->WriteLittleUint32(98765);
    sink->WriteCString("asd");
    sink->WriteCString(std::string{"def"});
    sink.reset();

    Byte exp[15] = { 247, 0xd2, 0x04, 0xcd, 0x81, 0x01, 0x00,
                     'a', 's', 'd', 0, 'd', 'e', 'f', 0 };
    char act[15];

    std::ifstream is{"tmp", std::ios_base::binary};
    is.read(act, 15);
    REQUIRE(is.good());
    REQUIRE(memcmp(exp, act, 15) == 0);

    is.get();
    REQUIRE(is.eof());
}

TEST_CASE("memory one write", "[MemorySink]")
{
    Byte buf[16] = {15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30};
    Byte buf2[16];
    {
        MemorySink sink{buf2, 16};
        sink.Write(buf);
    }

    REQUIRE(memcmp(buf, buf2, 16) == 0);
}

TEST_CASE("memory multiple writes", "[MemorySink]")
{
    Byte buf[8] = {42,43,44,45,46,47,48,49};
    Byte buf_out[32];
    Byte buf_exp[32];
    {
        MemorySink sink{buf_out, 32};
        for (size_t i = 0; i < 32; i+=8)
        {
            memcpy(buf_exp+i, buf, 8);
            sink.Write(buf);
            buf[0] = i;
        }
    }

    REQUIRE(memcmp(buf_out, buf_exp, 32) == 0);
}

TEST_CASE("memory pad", "[MemorySink]")
{
    Byte buf[8] = {77,78,79,80,81,82,83,84};
    Byte buf_out[32];
    Byte buf_exp[32];
    {
        MemorySink sink{buf_out, 32};
        memcpy(buf_exp, buf, 8);
        sink.Write(buf);

        memset(buf_exp+8, 0, 16);
        sink.Pad(16);

        memcpy(buf_exp+24, buf, 8);
        sink.Write(buf);
    }

    REQUIRE(memcmp(buf_out, buf_exp, 32) == 0);
}
