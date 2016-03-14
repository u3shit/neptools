#include "../format/item.hpp"
#include "../format/cl3.hpp"
#include "../format/stcm/file.hpp"
#include "../format/stcm/gbnl.hpp"
#include "../fs.hpp"
#include "../except.hpp"
#include "../utils.hpp"
#include "version.hpp"
#include <iostream>
#include <fstream>
#include <deque>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/exception/errinfo_file_name.hpp>

namespace
{

struct InvalidParameters {};
struct ParamError : public std::runtime_error
{ using std::runtime_error::runtime_error; };

struct Command
{
    using DoFun = std::function<void (std::deque<const char*>&)>;

    Command(std::string name, DoFun fun, std::string help)
        : name{std::move(name)}, fun{std::move(fun)}, help{std::move(help)} {}

    std::string name;
    DoFun fun;
    std::string help;
};

struct State
{
    std::unique_ptr<Dumpable> file;
    Cl3* cl3;
    Stcm::File* stcm;
    Gbnl* gbnl;
};

State SmartOpen_(const fs::path& fname)
{
    auto src = Source::FromFile(fname.native());
    if (src.GetSize() < 4)
        THROW(DecodeError{"Input file too short"});

    char buf[4];
    src.Pread(0, buf, 4);
    if (memcmp(buf, "CL3L", 4) == 0)
    {
        auto cl3 = std::make_unique<Cl3>(src);
        auto ret2 = cl3.get();
        return {std::move(cl3), ret2, nullptr, nullptr};
    }
    else if (memcmp(buf, "STCM", 4) == 0)
    {
        auto stcm = std::make_unique<Stcm::File>(src);
        auto ret2 = stcm.get();
        return {std::move(stcm), nullptr, ret2, nullptr};
    }
    else if (src.GetSize() >= sizeof(Gbnl::Header) &&
             (memcmp(buf, "GSTL", 4) == 0 ||
              (src.Pread(src.GetSize() - sizeof(Gbnl::Header), buf, 4),
               memcmp(buf, "GBNL", 4) == 0)))
    {
        auto gbnl = std::make_unique<Gbnl>(src);
        auto ret2 = gbnl.get();
        return {std::move(gbnl), nullptr, nullptr, ret2};
    }
    else
        THROW(DecodeError{"Unknown input file"});
}

State SmartOpen(const fs::path& fname)
{
    return AddInfo(
        SmartOpen_,
        [&](auto& e) { e << boost::errinfo_file_name{fname.string()}; },
        fname);
}

template <typename T>
void ShellDump(const T* item, std::deque<const char*>& args)
{
    if (args.empty()) throw ParamError{"missing argument"};

    auto fname = args.front(); args.pop_front();
    std::unique_ptr<Sink> sink;
    if (fname[0] == '-' && fname[1] == '\0')
        sink = Sink::ToStdOut();
    else
        sink = Sink::ToFile(fname, item->GetSize());
    item->Dump(*sink);
}

template <typename T, typename Fun>
void ShellInspectGen(const T* item, std::deque<const char*>& args, Fun f)
{
    if (args.empty()) throw ParamError{"missing argument"};

    auto fname = args.front(); args.pop_front();
    if (fname[0] == '-' && fname[1] == '\0')
        f(item, std::cout);
    else
        f(item, OpenOut(fname));
}

template <typename T>
void ShellInspect(const T* item, std::deque<const char*>& args)
{ ShellInspectGen(item, args, [](auto x, auto&& y) { y << *x; }); }

void EnsureStcm(State& st)
{
    if (st.stcm) return;
    if (!st.file) throw ParamError{"no file loaded"};
    if (!st.cl3)
        throw ParamError{"invalid file loaded: can't find STCM without CL3"};

    st.stcm = &st.cl3->GetStcm();
}

void EnsureGbnl(State& st)
{
    if (st.gbnl) return;
    EnsureStcm(st);
    st.gbnl = &st.stcm->FindGbnl();
}

bool auto_failed = false;
template <typename Pred, typename Fun>
void RecDo(const fs::path& path, Pred p, Fun f, bool rec = false)
{
    if (p(path, rec))
    {
        try { f(path); }
        catch (const std::exception& e)
        {
            auto_failed = true;
            std::cerr << "Failed: ";
            PrintException(std::cerr);
        }
    }
    else if (fs::is_directory(path))
        for (auto& e: fs::directory_iterator(path))
            RecDo(e, p, f, true);
    else if (!rec)
        std::cerr << "Invalid filename: " << path << std::endl;
}

enum class Mode
{
#define MODE_PARS(X)                                                            \
    X(AUTO_STRTOOL,   "auto-strtool",   "import/export .cl3/.gbin/.gstr texts") \
    X(EXPORT_STRTOOL, "export-strtool", "export .cl3/.gbin/.gstr to .txt")      \
    X(IMPORT_STRTOOL, "import-strtool", "import .cl3/.gbin/.gstr from .txt")    \
    X(AUTO_CL3,       "auto-cl3",       "unpack/pack .cl3 files")               \
    X(UNPACK_CL3,     "unpack-cl3",     "unpack .cl3 files")                    \
    X(PACK_CL3,       "pack-cl3",       "pack .cl3 files")                      \
    X(MANUAL,         "manual",         "manual processing (set automatically)")
#define GEN_ENUM(name, shit1, shit2) name,
    MODE_PARS(GEN_ENUM)
#undef GEN_ENUM
} mode = Mode::AUTO_STRTOOL;

void DoAutoFun(const fs::path& p)
{
    fs::path cl3, txt;
    bool import;
    if (boost::ends_with(p.native(), ".txt"))
    {
        cl3 = p.native().substr(0, p.native().size()-4);
        txt = p;
        import = true;
        std::cerr << "Importing: " << cl3 << " <- " << txt << std::endl;
    }
    else
    {
        cl3 = txt = p;
        txt += ".txt";
        import = false;
        std::cerr << "Exporting: " << cl3 << " -> " << txt << std::endl;
    }

    auto st = SmartOpen(cl3);
    EnsureGbnl(st);
    if (import)
    {
        st.gbnl->ReadTxt(OpenIn(txt));
        if (st.stcm) st.stcm->Fixup();
        st.file->Fixup();
        st.file->Dump(cl3);
    }
    else
        st.gbnl->WriteTxt(OpenOut(txt));
}

void DoAutoCl3(const fs::path& p)
{
    if (fs::is_directory(p))
    {
        fs::path cl3_file = p.native().substr(0, p.native().size() - 4);
        std::cerr << "Packing " << cl3_file << std::endl;
        Cl3 cl3{Source::FromFile(cl3_file)};
        cl3.UpdateFromDir(p);
        cl3.Fixup();
        cl3.Dump(cl3_file);
    }
    else
    {
        std::cerr << "Extracting " << p << std::endl;
        Cl3 cl3{Source::FromFile(p)};
        auto out = p;
        cl3.ExtractTo(out += ".out");
    }
}

inline bool is_file(const fs::path& pth)
{
    auto stat = fs::status(pth);
    return fs::is_regular_file(stat) || fs::is_symlink(stat);
}

bool IsBin(const fs::path& p, bool = false)
{
    return is_file(p) && (
        boost::ends_with(p.native(), ".cl3") ||
        boost::ends_with(p.native(), ".gbin") ||
        boost::ends_with(p.native(), ".gstr"));
}

bool IsTxt(const fs::path& p, bool = false)
{
    return is_file(p) && (
        boost::ends_with(p.native(), ".cl3.txt") ||
        boost::ends_with(p.native(), ".gbin.txt") ||
        boost::ends_with(p.native(), ".gstr.txt"));
}

bool IsCl3(const fs::path& p, bool = false)
{
    return is_file(p) && boost::ends_with(p.native(), ".cl3");
}

bool IsCl3Dir(const fs::path& p, bool = false)
{
    return fs::is_directory(p) && boost::ends_with(p.native(), ".cl3.out");
}

void DoAuto(const fs::path& path)
{
    bool (*pred)(const fs::path&, bool);
    void (*fun)(const fs::path& p);

    switch (mode)
    {
    case Mode::AUTO_STRTOOL:
        pred = [](auto& p, bool rec)
        {
            if (rec)
                return (IsTxt(p) &&
                        fs::exists(p.native().substr(0, p.native().size()-4))) ||
                       (IsBin(p) &&
                        !fs::exists(fs::path(p)+=".txt"));
            else
                return IsBin(p) || IsTxt(p);
        };
        fun = DoAutoFun;
        break;

    case Mode::EXPORT_STRTOOL:
        pred = IsBin;
        fun = DoAutoFun;
        break;
    case Mode::IMPORT_STRTOOL:
        pred = IsTxt;
        fun = DoAutoFun;
        break;

    case Mode::AUTO_CL3:
        pred = [](auto& p, bool rec)
        {
            if (rec)
                return IsCl3Dir(p) ||
                    (IsCl3(p) && !fs::exists(fs::path(p)+=".out"));
            else
                return IsCl3(p) || IsCl3Dir(p);
        };
        fun = DoAutoCl3;
        break;

    case Mode::UNPACK_CL3:
        pred = IsCl3;
        fun = DoAutoCl3;
        break;
    case Mode::PACK_CL3:
        pred = IsCl3Dir;
        fun = DoAutoCl3;
        break;

    case Mode::MANUAL:
        throw ParamError{"Can't use auto files in manual mode"};
    }
    RecDo(path, pred, fun);
}

}

int main(int argc, char** argv)
{
    State st;

    std::deque<const char*> args(argv+1, argv+argc);
    std::vector<Command> commands;
#define CMD(...) commands.emplace_back(__VA_ARGS__)
    CMD("--help", [](auto&){ throw InvalidParameters{}; }, "\n\tShow this help message\n");
#define GEN_IFS(c, str, _) else if (strcmp(args.front(), str) == 0) mode = Mode::c;
#define GEN_HELP(_, key, help) "\t\t" key ": " help "\n"
    CMD("--mode", [](auto& args)
        {
            if (args.empty()) throw ParamError{"missing argument"};
            if (0);
            MODE_PARS(GEN_IFS)
            else throw ParamError{"invalid parameter"};
            args.pop_front();
        },
        "\n\tSet operating mode:\n"
        MODE_PARS(GEN_HELP));
#undef GEN_IFS
#undef GEN_HELP

    CMD("--export-only", [&](auto&)
        {
            std::cerr << "--export-only deprecated, use --mode export-strtool\n";
            mode = Mode::EXPORT_STRTOOL;
        },
        "\n\tDeprecated, same as --mode export-strtool\n");
    CMD("--import-only", [&](auto&)
        {
            std::cerr << "--import-only deprecated, use --mode import-strtool\n";
            mode = Mode::IMPORT_STRTOOL;
        },
        "\n\tDeprecated, same as --mode import-strtool\n\nAdvanced operations (see README):\n");

    CMD("--open", [&](auto& args)
        {
            mode = Mode::MANUAL;
            if (args.empty()) throw ParamError{"missing argument"};
            st = SmartOpen(args.front());
            args.pop_front();
        }, "<file>\n\tOpens <file> as a cl3 or stcm file\n");
    CMD("--save", [&](auto& args)
        {
            mode = Mode::MANUAL;
            if (!st.file) throw ParamError{"no file loaded"};
            st.file->Fixup();
            ShellDump(st.file.get(), args);
        }, "<file>|-\n\tSaves the loaded file to <file> or stdout\n");
    CMD("--create-cl3", [&](auto&)
        {
            mode = Mode::MANUAL;
            auto c = std::make_unique<Cl3>();
            auto c2 = c.get();
            st = {std::move(c), c2, nullptr, nullptr};
        }, "\n\tCreates an empty cl3 file\n");
    CMD("--list-files", [&](auto&)
        {
            mode = Mode::MANUAL;
            if (!st.cl3) throw ParamError{"no cl3 loaded"};
            size_t i = 0;
            for (const auto& e : st.cl3->entries)
            {
                std::cout << i++ << '\t' << e.name << '\t' << e.src->GetSize()
                          << "\tlinks:";
                for (auto l : e.links) std::cout << ' ' << l;
                std::cout << std::endl;
            }
        }, "\n\tLists the contents of the cl3 archive\n");
    CMD("--extract-file", [&](auto& args) -> void
        {
            mode = Mode::MANUAL;
            if (args.size() < 2) throw ParamError{"missing arguments"};
            if (!st.cl3) throw ParamError{"no cl3 loaded"};
            auto e = st.cl3->GetFile(args.front()); args.pop_front();

            if (!e)
                throw ParamError{"specified file not found"};
            else
                ShellDump(e->src.get(), args);
        }, "<name> <out_file>|-\n\tExtract <name> from cl3 archive to <out_file> or stdout\n");
    CMD("--extract-files", [&](auto& args)
        {
            mode = Mode::MANUAL;
            if (args.empty()) throw ParamError{"missing argument"};
            if (!st.cl3) throw ParamError{"no cl3 loaded"};
            st.cl3->ExtractTo(args.front());
            args.pop_front();
        }, "<dir>\n\tExtract the cl3 archive to <dir>\n");
    CMD("--replace-file", [&](auto& args)
        {
            mode = Mode::MANUAL;
            if (args.size() < 2) throw InvalidParameters{};
            if (!st.cl3) throw ParamError{"no cl3 loaded"};

            auto& e = st.cl3->GetOrCreateFile(args[0]);
            e.src = std::make_unique<DumpableSource>(Source::FromFile(args[1]));

            args.pop_front(); args.pop_front();
        }, "<name> <in_file>\n\tAdds or replaces <name> in cl3 archive with <in_file>\n");
    CMD("--remove-file", [&](auto& args)
        {
            mode = Mode::MANUAL;
            if (args.empty()) throw ParamError{"missing argument"};
            if (!st.cl3) throw ParamError{"no cl3 loaded"};
            auto e = st.cl3->GetFile(args.front()); args.pop_front();
            if (!e)
                throw ParamError{"specified file not found"};
            else
                st.cl3->DeleteFile(*e);
        }, "<name>\n\tRemoves <name> from cl3 archive\n");
    CMD("--set-link", [&](auto& args)
        {
            mode = Mode::MANUAL;
            if (args.size() < 3) throw ParamError{"missing arguments"};
            if (!st.cl3) throw ParamError{"no cl3 loaded"};
            auto e = st.cl3->GetFile(args.front()); args.pop_front();
            auto i = std::stoul(args.front()); args.pop_front();
            auto e2 = st.cl3->GetFile(args[1]); args.pop_front();
            if (!e || !e2) throw ParamError{"specified file not found"};

            if (i < e->links.size())
                e->links[i] = e2 - &st.cl3->entries.front();
            else if (i == e->links.size())
                e->links.push_back(e2 - &st.cl3->entries.front());
            else
                throw ParamError{"invalid link id"};
        }, "<name> <id> <dst>\n\tSets link at <name>, <id> to <dst>");
    CMD("--remove-link", [&](auto& args)
        {
            mode = Mode::MANUAL;
            if (args.size() < 2) throw ParamError{"missing arguments"};
            if (!st.cl3) throw ParamError{"no cl3 loaded"};
            auto e = st.cl3->GetFile(args.front()); args.pop_front();
            auto i = std::stoul(args.front()); args.pop_front();
            if (!e) throw ParamError{"specified file not found"};

            if (i < e->links.size())
                e->links.erase(e->links.begin() + i);
            else
                throw ParamError{"invalid link id"};
        }, "<name> <id>\n\tRemove link <id> from <name>");
    CMD("--inspect", [&](auto& args)
        {
            mode = Mode::MANUAL;
            if (!st.file) throw ParamError{"No file loaded"};
            ShellInspect(st.file.get(), args);
        }, "<out>|-\n\tInspects currently loaded file into <out> or stdout\n");
    CMD("--inspect-stcm", [&](auto& args)
        {
            mode = Mode::MANUAL;
            EnsureStcm(st);
            ShellInspect(st.stcm, args);
        }, "<out>|-\n\tInspects only the stcm portion of the currently loaded file into <out> or stdout\n");
    CMD("--parse-stcm", [&](auto&)
        {
            mode = Mode::MANUAL;
            EnsureStcm(st);
        },
        "\n\tParse STCM-inside-CL3 (usually done automatically)\n");
    CMD("--export-txt", [&](auto& args) -> void
        {
            mode = Mode::MANUAL;
            EnsureGbnl(st);
            ShellInspectGen(st.gbnl, args, [](auto& x, auto&& y) { x->WriteTxt(y); });
        }, "<out_file>|-\n\tExport text to <out_file> or stdout\n");
    CMD("--import-txt", [&](auto& args)
        {
            mode = Mode::MANUAL;
            if (args.empty()) throw ParamError{"missing arguments"};
            EnsureGbnl(st);
            auto fname = args.front(); args.pop_front();
            if (fname[0] == '-' && fname[1] == '\0')
                st.gbnl->ReadTxt(std::cin);
            else
                st.gbnl->ReadTxt(OpenIn(fname));
            if (st.stcm) st.stcm->Fixup();
        }, "<in_file>|-\n\tRead text from <in_file> or stdin\n");

    fs::path self{argv[0]};
    if (boost::iequals(self.filename().string(), "cl3-tool")
#ifdef WINDOWS
        || boost::iequals(self.filename().string(), "cl3-tool.exe")
#endif
        )
        mode = Mode::AUTO_CL3;

    const char* cmd = nullptr;
    try
    {
        if (args.empty()) throw InvalidParameters{};

        while (!args.empty())
        {
            cmd = args.front(); args.pop_front();
            if (cmd[0] == '-' && cmd[1] == '-' && cmd[2] == '\0') break;

            if (cmd[0] == '-')
            {
                auto it = std::find_if(
                    commands.begin(), commands.end(),
                    [cmd](const auto& x) { return x.name == cmd; });
                if (it == commands.end())
                    throw ParamError{"unknown parameter"};

                it->fun(args);
            }
            else
                DoAuto(cmd);
        }
        while (!args.empty())
        {
            DoAuto(args.front());
            args.pop_front();
        }
    }
    catch (InvalidParameters ip)
    {
        std::cerr << "NepTools stcm-editor v" VERSION "\nUsage: " << argv[0]
                  << " [--options] [<file/directory>...]\n"
                     "Default operation: import all .cl3.txt to .cl3, export "
                     "all .cl3 to .cl3.txt.\n\n"
                     "Options:\n";

        for (auto& c : commands)
            std::cerr << "  " << c.name << ' ' << c.help;

        return -1;
    }
    catch (const ParamError& e)
    {
        std::cerr << cmd << ": " << e.what() << "\nRun " << argv[0]
                  << " --help to list available parameters" << std::endl;
        return 3;
    }
    catch (...)
    {
        std::cerr << "Fatal error, aborting\n";
        PrintException(std::cerr);
        return 4;
    }
    return auto_failed;;
}
