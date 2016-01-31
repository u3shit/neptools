#include "item.hpp"
#include "cl3/file.hpp"
#include "stcm/file.hpp"
#include <iostream>

#include "stcm/gbnl.hpp"

static Stcm::GbnlItem* FindGbnl(Item* root)
{
    if (!root) return nullptr;

    auto x = dynamic_cast<Stcm::GbnlItem*>(root);
    if (x) return x;

    x = FindGbnl(root->GetChildren());
    if (x) return x;

    return FindGbnl(root->GetNext());
}

static Stcm::GbnlItem* FindGbnl(Context& ctx)
{
    auto x = FindGbnl(ctx.GetRoot());
    if (!x)
        throw std::runtime_error("No GBNL found");
    return x;
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0]
                  << " [redump|inspect|write-txt|read-txt] file_name"
                  << std::endl;
        return 1;
    }

    if (strcmp(argv[1], "cl3") == 0)
    {
        Cl3::File ctx{argv[2]};
        std::cout << ctx;
        return 0;
    }

    Stcm::File ctx{argv[2]};

    if (strcmp(argv[1], "inspect") == 0)
        std::cout << ctx;
    else if (strcmp(argv[1], "write-txt") == 0)
        FindGbnl(ctx)->WriteTxt(std::cout);
    else if (strcmp(argv[1], "read-txt") == 0)
        FindGbnl(ctx)->ReadTxt(std::cin);

    if (strcmp(argv[1], "redump") == 0 || strcmp(argv[1], "read-txt") == 0)
    {
        /*
        for (auto it = ctx.GetRoot(); it; )
        {
            auto next = it->GetNext();
            if (dynamic_cast<RawItem*>(it) && it->GetLabels().empty())
                it->Remove();
            it = next;
        }
        */

        ctx.UpdatePositions();
        ctx.Dump(std::cout);
    }
}
