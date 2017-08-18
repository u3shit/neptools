#include <libshit/options.hpp>
#include <catch.hpp>
#include <cstring>

#pragma GCC diagnostic ignored "-Waddress"

using namespace Libshit;

static void Run(OptionParser& parser, int& argc, const char** argv, bool val)
{
  try
  {
    parser.Run(argc, argv);
    FAIL("Expected parser to throw Exit, but didn't throw anything");
  }
  catch (const Exit& exit)
  {
    CHECK(exit.success == val);
  }
}

#define CHECK_STREQ(a, b)                           \
  do                                                \
  {                                                 \
    if (a == nullptr || b == nullptr)               \
      CHECK(a == b);                                \
    else                                            \
    {                                               \
      INFO(#a " = " << a << "\n" #b << " = " << b); \
      CHECK(strcmp(a, b) == 0);                     \
    }                                               \
  }                                                 \
  while (0)

TEST_CASE("basic option parsing", "[Options]")
{
  std::stringstream ss;
  OptionParser parser;
  parser.SetOstream(ss);
  OptionGroup grp{parser, "Foo", "Description of foo"};

  bool b1 = false, b2 = false;
  const char* b3 = nullptr, *b40 = nullptr, *b41 = nullptr;

  Option test1{grp, "test-1", 't', 0, nullptr, "foo", [&](auto){ b1 = true; }};
  Option test2{grp, "test-2", 'T', 0, nullptr, "bar", [&](auto){ b2 = true; }};
  Option test3{grp, "test-3", 1, "STRING", "Blahblah",
      [&](auto v){ REQUIRE(v.size() == 1); b3 = v[0]; }};
  Option test4{grp, "test-4", 'c', 2, "FOO BAR", nullptr,
      [&](auto v){ REQUIRE(v.size() == 2); b40 = v[0]; b41 = v[1]; }};

  bool e1 = false, e2 = false;
  const char* e3 = nullptr, *e40 = nullptr, *e41 = nullptr;

  const char* help_text =
    "General options:\n"
    " -h --help\n"
    "\tShow this help message\n"
    "\n"
    "Foo:\n"
    "Description of foo\n"
    " -t --test-1\n"
    "\tfoo\n"
    " -T --test-2\n"
    "\tbar\n"
    "    --test-3=STRING\n"
    "\tBlahblah\n"
    " -c --test-4=FOO BAR\n";

  int argc = 2;
  const char* argv[10] = { "foo" };

  SECTION("success")
  {
    SECTION("single option")
    {
      argv[1] = "--test-2";
      parser.Run(argc, argv);
      e2 = true;
    }

    SECTION("multiple options")
    {
      argc = 4;
      argv[1] = "--test-2";
      argv[2] = "--test-1";
      argv[3] = "--test-2";
      parser.Run(argc, argv);

      e1 = e2 = true;
    }

    SECTION("argument options")
    {
      SECTION("= arguments")
      {
        argc = 4;
        argv[1] = "--test-4=xx";
        argv[2] = "yy";
        argv[3] = "--test-3=foo";
      }
      SECTION("separate argument options")
      {
        argc = 6;
        argv[1] = "--test-4";
        argv[2] = "xx";
        argv[3] = "yy";
        argv[4] = "--test-3";
        argv[5] = "foo";
      }
      parser.Run(argc, argv);

      e3 = "foo";
      e40 = "xx";
      e41 = "yy";
    }

    SECTION("short argument")
    {
      argv[1] = "-t";
      parser.Run(argc, argv);
      e1 = true;
    }

    SECTION("short arguments concat")
    {
      argv[1] = "-tT";
      parser.Run(argc, argv);
      e1 = e2 = true;
    }

    SECTION("short arguments parameter")
    {
      SECTION("normal")
      {
        argc = 4;
        argv[1] = "-tc";
        argv[2] = "abc";
        argv[3] = "def";
      }
      SECTION("concat")
      {
        argc = 3;
        argv[1] = "-tcabc";
        argv[2] = "def";
      }
      parser.Run(argc, argv);
      e1 = true;
      e40 = "abc";
      e41 = "def";
    }

    REQUIRE(argc == 1);
    CHECK(argv[0] == "foo");
    CHECK(argv[1] == nullptr);
  }

  SECTION("unused args")
  {
    argc = 5;
    argv[1] = "--test-1";
    argv[2] = "foopar";
    argv[3] = "barpar";
    argv[4] = "-T";

    parser.Run(argc, argv);
    e1 = e2 = true;
    REQUIRE(argc == 3);
    CHECK(argv[0] == "foo");
    CHECK(argv[1] == "foopar");
    CHECK(argv[2] == "barpar");
  }

  SECTION("unused -- teminating")
  {
    argc = 4;
    argv[1] = "-t";
    argv[2] = "--";
    argv[3] = "-T";
    parser.Run(argc, argv);
    e1 = true;

    REQUIRE(argc == 2);
    CHECK(argv[0] == "foo");
    CHECK(argv[1] == "-T");
    CHECK(argv[2] == nullptr);
  }

  SECTION("unused handler")
  {
    argc = 6;
    argv[1] = "-t";
    argv[2] = "foopar";
    argv[3] = "-T";
    argv[4] = "--";
    argv[5] = "--help";

    std::vector<const char*> vec;
    parser.SetNoArgHandler([&](auto x) { vec.push_back(x); });
    parser.Run(argc, argv);
    e1 = e2 = true;

    CHECK(vec == (std::vector<const char*>{"foopar", "--help"}));
    REQUIRE(argc == 1);
    CHECK(argv[0] == "foo");
    CHECK(argv[1] == nullptr);
  }

  SECTION("empty args")
  {
    argc = 1;
    parser.Run(argc, argv);
    REQUIRE(argc == 1);
    CHECK(argv[0] == "foo");
    CHECK(argv[1] == nullptr);
  }

  SECTION("empty show help")
  {
    argc = 1;
    parser.SetShowHelpOnNoOptions();
    Run(parser, argc, argv, true);
    CHECK(ss.str() == help_text);
  }

  SECTION("show help")
  {
    SECTION("normal") { argv[1] = "--help"; }
    SECTION("abbrev") { argv[1] = "--he"; }
    Run(parser, argc, argv, true);
    CHECK(ss.str() == help_text);
  }

  SECTION("ambiguous option")
  {
    argv[1] = "--test";
    Run(parser, argc, argv, false);
    CHECK(ss.str() ==
          "--test: Ambiguous option (candidates: --test-1 --test-2 --test-3 --test-4)\n");
  }

  SECTION("ambiguous option with params")
  {
    argv[1] = "--test=foo";
    Run(parser, argc, argv, false);
    CHECK(ss.str() ==
          "--test=foo: Ambiguous option (candidates: --test-1 --test-2 --test-3 --test-4)\n");
  }

  SECTION("unknown option")
  {
    argv[1] = "--foo";
    Run(parser, argc, argv, false);
    CHECK(ss.str() == "--foo: Unknown option\n");
  }

  SECTION("unknown short option")
  {
    argv[1] = "-x";
    Run(parser, argc, argv, false);
    CHECK(ss.str() == "-x: Unknown option\n");
  }

  CHECK(b1 == e1);
  CHECK(b2 == e2);
  CHECK_STREQ(b3, e3);
  CHECK_STREQ(b40, e40);
  CHECK_STREQ(b41, e41);
}

TEST_CASE("version displaying", "[Options]")
{
  std::stringstream ss;
  OptionParser parser;
  parser.SetOstream(ss);
  parser.SetVersion("Foo program bar");

  int argc = 2;
  const char* argv[] = { "foo", "--help", nullptr };

  SECTION("help message")
  {
    Run(parser, argc, argv, true);
    CHECK(ss.str() ==
          "Foo program bar\n"
          "\n"
          "General options:\n"
          " -h --help\n"
          "\tShow this help message\n"
          "    --version\n"
          "\tShow program version\n");
  }
  SECTION("version display")
  {
    argv[1] = "--ver";
    Run(parser, argc, argv, true);
    CHECK(ss.str() == "Foo program bar\n");
  }
}

TEST_CASE("usage displaying", "[Options]")
{
  std::stringstream ss;
  OptionParser parser;
  parser.SetOstream(ss);
  parser.SetUsage("[--options] [bar...]");

  int argc = 2;
  const char* argv[] = { "foo", "--help", nullptr };

  Run(parser, argc, argv, true);
  CHECK(ss.str() ==
        "Usage:\n"
        "\tfoo [--options] [bar...]\n"
        "\n"
        "General options:\n"
        " -h --help\n"
        "\tShow this help message\n");
}

TEST_CASE("non-unique prefix", "[Options]")
{
  std::stringstream ss;
  OptionParser parser;
  parser.SetOstream(ss);

  OptionGroup grp{parser, "Foo", "Description of foo"};

  bool b1 = false, b2 = false;

  Option test1{grp, "foo",     0, nullptr, "foo", [&](auto){ b1 = true; }};
  Option test2{grp, "foo-bar", 0, nullptr, "bar", [&](auto){ b2 = true; }};

  int argc = 2;
  const char* argv[] = { "foo", "--foo", nullptr };

  parser.Run(argc, argv);
  CHECK(ss.str() == "");
  CHECK(b1 == true);
  CHECK(b2 == false);
}

TEST_CASE("non-unique prefix with options", "[Options]")
{
  std::stringstream ss;
  OptionParser parser;
  parser.SetOstream(ss);

  OptionGroup grp{parser, "Foo", "Description of foo"};

  const char* b1 = nullptr;
  bool b2 = false;

  Option test1{grp, "foo",     1, nullptr, "foo", [&](auto a){ b1 = a.front(); }};
  Option test2{grp, "foo-bar", 0, nullptr, "bar", [&](auto){ b2 = true; }};

  int argc;
  const char* argv[4] = {"foo"};
  SECTION("space separated")
  {
    argc = 3;
    argv[1] = "--foo";
    argv[2] = "bar";
  }
  SECTION("= separated")
  {
    argc = 2;
    argv[1] = "--foo=bar";
  }

  parser.Run(argc, argv);
  CHECK(ss.str() == "");
  CHECK_STREQ(b1, "bar");
  CHECK(b2 == false);
}
