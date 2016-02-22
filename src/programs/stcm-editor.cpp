#include "../format/item.hpp"
#include "../format/context_item.hpp"
#include "../format/cl3/file.hpp"
#include "../format/cl3/file_collection.hpp"
#include "../format/stcm/file.hpp"
#include "../format/stcm/gbnl.hpp"
#include "../fs.hpp"
#include "../utils.hpp"
#include <iostream>
#include <fstream>
#include <deque>
#include <boost/algorithm/string/predicate.hpp>

namespace
{

struct InvalidParameters {};

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
    Cl3::File* cl3;
    Stcm::File* stcm;
    Gbnl* gbnl;
};

State SmartOpen(const fs::path& fname)
{
    auto buf = ReadFile(fname);
    if (buf->GetSize() < 4)
        throw std::runtime_error("Input file too short");

    if (memcmp(buf->GetPtr(), "CL3L", 4) == 0)
    {
        auto cl3 = std::make_unique<Cl3::File>(buf);
        auto ret2 = cl3.get();
        return {std::move(cl3), ret2, nullptr, nullptr};
    }
    else if (memcmp(buf->GetPtr(), "STCM", 4) == 0)
    {
        auto stcm = std::make_unique<Stcm::File>(buf);
        auto ret2 = stcm.get();
        return {std::move(stcm), nullptr, ret2, nullptr};
    }
    else if (buf->GetSize() >= sizeof(GbnlFooter) &&
             (memcmp(buf->GetPtr(), "GSTL", 4) == 0 ||
              memcmp(buf->GetPtr() + buf->GetSize() - sizeof(GbnlFooter),
                     "GBNL", 4) == 0))
    {
        auto gbnl = std::make_unique<Gbnl>(buf.get());
        auto ret2 = gbnl.get();
        return {std::move(gbnl), nullptr, nullptr, ret2};
    }
    else
        throw std::runtime_error{"Unknown input file"};
}

template <typename T, typename Fun>
void ShellDumpGen(const T* item, std::deque<const char*>& args, Fun f)
{
    if (args.empty()) throw InvalidParameters{};

    auto fname = args.front(); args.pop_front();
    if (fname[0] == '-' && fname[1] == '\0')
        f(item, std::cout);
    else
    {
        f(item, OpenOut(fname));
    }
}

template <typename T>
void ShellDump(const T* item, std::deque<const char*>& args)
{ ShellDumpGen(item, args, [](auto x, auto&& y) { x->Dump(y); }); }

template <typename T>
void ShellInspect(const T* item, std::deque<const char*>& args)
{ ShellDumpGen(item, args, [](auto x, auto&& y) { y << *x; }); }

void EnsureStcm(State& st)
{
    if (st.stcm) return;
    if (!st.file) throw std::runtime_error{"No file loaded"};
    if (!st.cl3)
        throw std::runtime_error{"Invalid file loaded: can't find STCM without CL3"};

    auto dat = st.cl3->GetFileCollection().GetFile("main.DAT");
    if (!dat) throw std::runtime_error{"Invalid CL3 file: no main.DAT"};

    auto ritem = asserted_cast<RawItem*>(dat->GetChildren());
    auto nstcm = st.cl3->Create<ContextItem<Stcm::File>>(
        ritem->GetBuffer(), ritem->GetOffset(), ritem->GetSize());
    st.stcm = nstcm.get();
    ritem->Replace(std::move(nstcm));
}

Stcm::GbnlItem* FindGbnl(Item* root)
{
    if (!root) return nullptr;

    auto x = dynamic_cast<Stcm::GbnlItem*>(root);
    if (x) return x;

    x = FindGbnl(root->GetChildren());
    if (x) return x;

    return FindGbnl(root->GetNext());
}

void EnsureGbnl(State& st)
{
    if (st.gbnl) return;
    EnsureStcm(st);

    st.gbnl = FindGbnl(st.stcm->GetRoot());
    if (!st.gbnl)
        throw std::runtime_error{"No GBNL found in STCM"};
}

template <typename Pred, typename Fun>
void RecDo(const fs::path& path, Pred p, Fun f, bool rec = false)
{
    if (fs::is_directory(path))
        for (auto& e: fs::directory_iterator(path))
            RecDo(e, p, f, true);
    else if (p(path, rec))
    {
        try { f(path); }
        catch (const std::runtime_error& e)
        {
            std::cerr << "Failed: " << e.what() << std::endl;
        }
    }
    else if (!rec)
        std::cerr << "Invalid filename: " << path << std::endl;
}

bool do_import = true, do_export = true;
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

bool IsBin(const fs::path& p, bool = false)
{
    return boost::ends_with(p.native(), ".cl3") ||
        boost::ends_with(p.native(), ".gbin") ||
        boost::ends_with(p.native(), ".gstr");
}

bool IsTxt(const fs::path& p, bool = false)
{
    return boost::ends_with(p.native(), ".cl3.txt") ||
        boost::ends_with(p.native(), ".gbin.txt") ||
        boost::ends_with(p.native(), ".gstr.txt");
}

void DoAuto(const fs::path& path)
{
    bool (*pred)(const fs::path&, bool);

    if (do_import && do_export)
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
    else if (do_export)
        pred = IsBin;
    else
        pred = IsTxt;
    RecDo(path, pred, DoAutoFun);
}

}

int main(int argc, char** argv)
{
    State st;

    std::deque<const char*> args(argv+1, argv+argc);
    std::vector<Command> commands;
#define CMD(...) commands.emplace_back(__VA_ARGS__)
    CMD("--help", [](auto&){ throw InvalidParameters{}; }, "\n\tShow this help message\n");
    CMD("--export-only", [&](auto&) { do_import = false; do_export = true; },
        "\n\tOnly export txt (.cl3->.cl3.txt)\n");
    CMD("--import-only", [&](auto&) { do_import = true; do_export = false; },
        "\n\tOnly import txt (.cl3.txt->.cl3)\n\nAdvanced operations (see README):\n");

    CMD("--open", [&](auto& args)
        {
            if (args.empty()) throw InvalidParameters{};
            st = SmartOpen(args.front());
            args.pop_front();
        }, "<file>\n\tOpens <file> as a cl3 or stcm file\n");
    CMD("--save", [&](auto& args)
        {
            if (!st.file) throw std::runtime_error{"--save: No file loaded"};
            st.file->Fixup();
            ShellDump(st.file.get(), args);
        }, "<file>|-\n\tSaves the loaded file to <file> or stdout\n");
    CMD("--create-cl3", [&](auto&)
        {
            auto c = std::make_unique<Cl3::File>();
            auto c2 = c.get();
            st = {std::move(c), c2, nullptr, nullptr};
        }, "\n\tCreates an empty cl3 file\n");
    CMD("--list-files", [&](auto&)
        {
            if (!st.cl3)
                throw std::runtime_error{"--list-files: No cl3 loaded"};
            for (const auto& e : st.cl3->GetFileCollection().entries)
                std::cout << e.name << '\t' << e.data->second.item->GetSize() << '\n';
        }, "\n\tLists the contents of the cl3 archive\n");
    CMD("--extract-file", [&](auto& args) -> void
        {
            if (args.size() < 2) throw InvalidParameters{};
            if (!st.cl3)
                throw std::runtime_error{"--extract-file: No cl3 loaded"};
            const auto& col = st.cl3->GetFileCollection().entries;
            auto fn = args.front(); args.pop_front();
            auto it = std::find_if(col.begin(), col.end(),
                                   [fn](auto& e) { return e.name == fn; });
            if (it == col.end())
                throw std::runtime_error{"--extract-file: specified file not found"};
            else
                ShellDump(it->data->second.item, args);
        }, "<name> <out_file>|-\n\tExtract <name> from cl3 archive to <out_file> or stdout\n");
    CMD("--extract-files", [&](auto& args)
        {
            if (args.empty()) throw InvalidParameters{};
            if (!st.cl3)
                throw std::runtime_error{"--extract-file: No cl3 loaded"};
            st.cl3->GetFileCollection().ExtractTo(args.front());
            args.pop_front();
        }, "<dir>\n\tExtract the cl3 archive to <dir>\n");
    CMD("--replace-file", [&](auto& args)
        {
            if (args.size() < 2) throw InvalidParameters{};
            if (!st.cl3)
                throw std::runtime_error{"--replace-file: No cl3 loaded"};
            auto& fc = st.cl3->GetFileCollection();

            fc.ReplaceFile(
                args[0], fc.GetContext()->Create<RawItem>(ReadFile(args[1])));
            args.pop_front(); args.pop_front();
        }, "<name> <in_file>\n\tAdds or replaces <name> in cl3 archive with <in_file>\n");
    CMD("--inspect", [&](auto& args)
        {
            if (!st.file) throw std::runtime_error{"--inspect: No file loaded"};
            ShellInspect(st.file.get(), args);
        }, "<out>|-\n\tInspects currently loaded file into <out> or stdout\n");
    CMD("--inspect-stcm", [&](auto& args)
        {
            EnsureStcm(st);
            ShellInspect(st.stcm, args);
        }, "<out>|-\n\tInspects only the stcm portion of the currently loaded file into <out> or stdout\n");
    CMD("--parse-stcm", [&](auto&) { EnsureStcm(st); },
        "\n\tParse STCM-inside-CL3 (usually done automatically)\n");
    CMD("--export-txt", [&](auto& args) -> void
        {
            EnsureGbnl(st);
            ShellDumpGen(st.gbnl, args, [](auto& x, auto&& y) { x->WriteTxt(y); });
        }, "<out_file>|-\n\tExport text to <out_file> or stdout\n");
    CMD("--import-txt", [&](auto& args)
        {
            if (args.empty()) throw InvalidParameters{};
            EnsureGbnl(st);
            auto fname = args.front(); args.pop_front();
            if (fname[0] == '-' && fname[1] == '\0')
                st.gbnl->ReadTxt(std::cin);
            else
                st.gbnl->ReadTxt(OpenIn(fname));
            if (st.stcm) st.stcm->Fixup();
        }, "<in_file>|-\n\tRead text from <in_file> or stdin\n");

    try
    {
        if (args.empty()) throw InvalidParameters{};

        while (!args.empty())
        {
            auto cmd = args.front(); args.pop_front();
            if (cmd[0] == '-' && cmd[1] == '-' && cmd[2] == '\0') break;

            if (cmd[0] == '-')
            {
                auto it = std::find_if(
                    commands.begin(), commands.end(),
                    [cmd](const auto& x) { return x.name == cmd; });
                if (it == commands.end()) throw InvalidParameters{};

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
        std::cerr << "Usage: " << argv[0]
                  << " [--options] [<file/directory>...]\n"
                     "Default operation: import all .cl3.txt to .cl3, export "
                     "all .cl3 to .cl3.txt.\n\n"
                     "Options:\n";

        for (auto& c : commands)
            std::cerr << "  " << c.name << ' ' << c.help;

        return -1;
    }
    return 0;
}