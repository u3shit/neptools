#include "item.hpp"
#include "context_item.hpp"
#include "cl3/file.hpp"
#include "cl3/file_collection.hpp"
#include "stcm/file.hpp"
#include "stcm/gbnl.hpp"
#include <iostream>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace fs = boost::filesystem;

namespace
{

struct InvalidParameters {};

struct Command
{
    virtual void Do(int argc, char** argv) = 0;
    virtual void Help(std::ostream& os) const = 0;
    virtual bool Hidden() const noexcept { return false; }
};

struct Cl3List : public Command
{
    void Do(int argc, char** argv) override
    {
        if (argc != 1) throw InvalidParameters{};

        Cl3::File file{argv[0]};
        for (const auto& e : file.GetFileCollection().entries)
            std::cout << e.name << '\t' << e.data->second.item->GetSize() << '\n';
    }

    void Help(std::ostream& os) const override
    { os << "<cl3_file>\n\tList Cl3 contents\n"; }
};

struct Cl3Inspect : public Command
{
    void Do(int argc, char** argv) override
    {
        if (argc != 1) throw InvalidParameters{};

        Cl3::File file{argv[0]};
        std::cout << file;
    }

    void Help(std::ostream& os) const override
    { os << "<cl3_file>\n\tInspect Cl3 file\n"; }
    bool Hidden() const noexcept override { return true; }
};

struct Cl3Extract : public Command
{
    void Do(int argc, char** argv) override
    {
        if (argc != 2) throw InvalidParameters{};

        Cl3::File file{argv[0]};
        file.GetFileCollection().ExtractTo(argv[1]);
    }
    void Help(std::ostream& os) const override
    { os << "<cl3_file> <output_dir>\n\tExtract Cl3 file\n"; }
};

void UpdateCl3(Cl3::File& file, int argc, char** argv)
{
    auto& fc = file.GetFileCollection();

    for (int i = 1; i < argc; i += 2)
        fc.ReplaceFile(
            argv[i], fc.GetContext()->Create<RawItem>(ReadFile(argv[i+1])));
    file.Fixup();
    file.Dump(argv[0]);
}

struct Cl3Update : public Command
{
    void Do(int argc, char** argv) override
    {
        if (argc < 2 || argc % 2 != 0) throw InvalidParameters{};
        Cl3::File file{argv[0]};
        UpdateCl3(file, argc-1, argv+1);
    }

    void Help(std::ostream& os) const override
    {
        os << "<cl3_input> <cl3_output> [<entry_name> <replacement_file>]..."
           << "\n\tReplaces or adds files into a Cl3 archive\n";
    }
};

struct Cl3Create : public Command
{
    void Do(int argc, char** argv) override
    {
        if (argc < 1 || argc % 2 != 1) throw InvalidParameters{};
        Cl3::File file;
        UpdateCl3(file, argc, argv);
        std::cout << file;
    }

    void Help(std::ostream& os) const override
    {
        os << "<cl3_output> [<entry_name> <replacement_file>]..."
           << "\n\tCreates a Cl3 archive from files\n";
    }
};

std::pair<std::unique_ptr<Context>, Stcm::File*>
SmartStcm(const fs::path& fname)
{
    auto buf = ReadFile(fname);
    if (buf->GetSize() < 4)
        throw std::runtime_error("Input file too short");

    if (memcmp(buf->GetPtr(), "CL3L", 4) == 0)
    {
        auto cl3 = std::make_unique<Cl3::File>(buf);
        auto dat = cl3->GetFileCollection().GetFile("main.DAT");
        if (!dat) throw std::runtime_error("Invalid CL3 file");

        BOOST_ASSERT(dynamic_cast<RawItem*>(dat->GetChildren()));
        auto ritem = static_cast<RawItem*>(dat->GetChildren());
        auto stcm = cl3->Create<ContextItem<Stcm::File>>(
            ritem->GetBuffer(), ritem->GetOffset(), ritem->GetSize());
        auto ret2 = stcm.get();
        ritem->Replace(std::move(stcm));
        return {std::move(cl3), ret2};
    }
    else if (memcmp(buf->GetPtr(), "STCM", 4) == 0)
    {
        auto stcm = std::make_unique<Stcm::File>(buf);
        auto ret2 = stcm.get();
        return {std::move(stcm), ret2};
    }
    else
        throw std::runtime_error("Invalid input file");
}

struct StcmInspect : public Command
{
    void Do(int argc, char** argv) override
    {
        if (argc != 1) throw InvalidParameters{};

        auto x = SmartStcm(argv[0]);
        std::cout << *x.first;
    }

    void Help(std::ostream& os) const override
    { os << "<cl3/stcm_file>\n\t"
            "Inspect Stcm file (possibly inside Cl3 as main.DAT)\n"; }
    bool Hidden() const noexcept override { return true; }
};

struct StcmRedump : public Command
{
    void Do(int argc, char** argv) override
    {
        if (argc != 2) throw InvalidParameters{};

        auto x = SmartStcm(argv[0]);
        x.first->Fixup();
        x.first->Dump(argv[1]);
    }

    void Help(std::ostream& os) const override
    { os << "<cl3/stcm_input> <cl3/stcm_output>\n\t"
            "Re-dumps Stcm file (possibly inside Cl3 as main.DAT)\n"; }
    bool Hidden() const noexcept override { return true; }
};

Stcm::GbnlItem* FindGbnl(Item* root)
{
    if (!root) return nullptr;

    auto x = dynamic_cast<Stcm::GbnlItem*>(root);
    if (x) return x;

    x = FindGbnl(root->GetChildren());
    if (x) return x;

    return FindGbnl(root->GetNext());
}

Stcm::GbnlItem* FindGbnl(Context& ctx)
{
    auto x = FindGbnl(ctx.GetRoot());
    if (!x)
        throw std::runtime_error("No GBNL found");
    return x;
}

template <typename Fun>
void RecDo(const fs::path& path, const std::string& ext, Fun f)
{
    if (fs::is_directory(path))
        for (auto& e: fs::directory_iterator(path))
            RecDo(e, ext, f);
    else if (boost::ends_with(path.native(), ext))
    {
        std::cerr << "Processing: " << path << std::endl;
        try { f(path); }
        catch (const std::runtime_error& e)
        {
            std::cerr << "Failed: " << e.what() << std::endl;
        }
    }
}

struct GbnlWrite : public Command
{
    static void Conv(const fs::path& in, const fs::path& out)
    {
        auto x = SmartStcm(in);
        auto gbnl = FindGbnl(*x.second);

        fs::ofstream os;
        os.exceptions(std::ios_base::failbit | std::ios_base::badbit);
        os.open(out, std::ios_base::out | std::ios_base::binary);
        gbnl->WriteTxt(os);
    }

    void Do(int argc, char** argv) override
    {
        if (argc == 1)
            RecDo(argv[0], ".cl3", [](auto& x) { Conv(x, fs::path(x)+=".txt"); });
        else if (argc == 2)
            Conv(argv[0], argv[1]);
        else
            throw InvalidParameters{};
    }
    void Help(std::ostream& os) const override
    { os << "[<dir>|<cl3/stcm_input> <txt_output>]\n\t"
            "1st variant: Extracts all *.cl3 files in <dir> into *.cl3.txt\n\t"
            "2nd variant: Extract messages from <cl3/stcm_input> into <txt_output>\n"; }
};

struct GbnlRead : public Command
{
    static void Conv(const fs::path& dat, const fs::path& txt, const fs::path& out)
    {
        auto x = SmartStcm(dat);
        {
            fs::ifstream is;
            is.exceptions(std::ios_base::failbit | std::ios_base::badbit);
            is.open(txt, std::ios_base::in | std::ios_base::binary);
            FindGbnl(*x.second)->ReadTxt(is);
        }
        x.first->Fixup();
        x.first->Dump(out);
    }

    void Do(int argc, char** argv) override
    {
        if (argc == 1)
            RecDo(argv[0], ".cl3.txt", [](auto& x) {
                fs::path dat = x.native().substr(0, x.size()-4);
                Conv(dat, x, dat);
            });
        else if (argc == 3)
            Conv(argv[0], argv[1], argv[2]);
        else
            throw InvalidParameters{};

    }
    void Help(std::ostream& os) const override
    { os << "[<dir>|<cl3/stcm_input> <txt_input> <cl3/stcm_output>]\n\t"
            "1st variant: Replaces messages in all *.cl3 files in <dir> from *.cl3.txt\n\t"
            "2nd variant: Replaces messages inside <cl3/stcm_input> with <txt_input>, storing result inside <cl3/stcm_output\n"; }
};

}

int main(int argc, char** argv)
{
    std::map<std::string, std::unique_ptr<Command>> commands;
    commands["cl3-list"] = std::make_unique<Cl3List>();
    commands["cl3-inspect"] = std::make_unique<Cl3Inspect>();
    commands["cl3-extract"] = std::make_unique<Cl3Extract>();
    commands["cl3-update"] = std::make_unique<Cl3Update>();
    commands["cl3-create"] = std::make_unique<Cl3Create>();

    commands["stcm-inspect"] = std::make_unique<StcmInspect>();
    commands["stcm-redump"] = std::make_unique<StcmRedump>();

    commands["write-txt"] = std::make_unique<GbnlWrite>();
    commands["read-txt"] = std::make_unique<GbnlRead>();

    bool show_hidden = false;
    int argc_base = 2;

    try
    {
        if (argc < 2) throw InvalidParameters{};
        if (strcmp(argv[1], "--show-hidden") == 0)
        {
            show_hidden = true;
            ++argc_base;
        }

        if (argc < argc_base) throw InvalidParameters{};
        auto it = commands.find(argv[argc_base-1]);
        if (it == commands.end()) throw InvalidParameters{};

        it->second->Do(argc-argc_base, argv+argc_base);
    }
    catch (InvalidParameters ip)
    {
        std::cerr << "Usage:\n";
        for (auto& c : commands)
            if (show_hidden || !c.second->Hidden())
            {
                std::cerr << argv[0] << ' ' << c.first << ' ';
                c.second->Help(std::cerr);
            }
        return -1;
    }
    return 0;
}
