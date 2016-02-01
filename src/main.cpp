#include "item.hpp"
#include "cl3/file.hpp"
#include "cl3/file_collection.hpp"
#include "stcm/file.hpp"
#include "stcm/gbnl.hpp"
#include <iostream>
#include <boost/filesystem/fstream.hpp>

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
    fc.RedoPadding();
    file.UpdatePositions();
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

struct StcmInspect : public Command
{
    void Do(int argc, char** argv) override
    {
        if (argc != 1) throw InvalidParameters{};

        Stcm::File file{argv[0]};
        std::cout << file;
    }

    void Help(std::ostream& os) const override
    { os << "<stcm_file>\n\tInspect Stcm file\n"; }
    bool Hidden() const noexcept override { return true; }
};

struct StcmRedump : public Command
{
    void Do(int argc, char** argv) override
    {
        if (argc != 2) throw InvalidParameters{};

        Stcm::File file{argv[0]};
        file.UpdatePositions();
        file.Dump(argv[1]);
    }

    void Help(std::ostream& os) const override
    { os << "<stcm_input> <stcm_output>\n\tRe-dumps Stcm file\n"; }
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

struct GbnlWrite : public Command
{
    void Do(int argc, char** argv) override
    {
        if (argc != 2) throw InvalidParameters{};

        Stcm::File file{argv[0]};
        boost::filesystem::ofstream os;
        os.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        os.open(argv[1], std::ios_base::out | std::ios_base::binary);
        FindGbnl(file)->WriteTxt(os);
    }
    void Help(std::ostream& os) const override
    { os << "<stcm_input> <txt_output>\n\tExtract messages from Stcm file\n"; }
};

struct GbnlRead : public Command
{
    void Do(int argc, char** argv) override
    {
        if (argc != 3) throw InvalidParameters{};

        Stcm::File file{argv[0]};

        {
            boost::filesystem::ifstream is;
            is.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            is.open(argv[1], std::ios_base::out | std::ios_base::binary);
            FindGbnl(file)->ReadTxt(is);
        }
        file.UpdatePositions();
        file.Dump(argv[2]);
    }
    void Help(std::ostream& os) const override
    { os << "<stcm_input> <txt_input> <stcm_output>\n\t"
            "Replaces messages inside Stcm file\n"; }
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

    commands["save-txt"] = std::make_unique<GbnlWrite>();
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
