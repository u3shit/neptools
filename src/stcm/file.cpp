#include "file.hpp"
#include "header.hpp"
#include "exports.hpp"
#include "instruction.hpp"
#include "../item.hpp"
#include <boost/assert.hpp>
#include <set>

namespace Stcm
{

File::File(std::shared_ptr<Buffer> buf)
{
    SetRoot(Create<RawItem>(std::move(buf)));
    Parse();
}

static const std::set<uint32_t> no_returns{0, 6};

void File::Parse()
{
    BOOST_ASSERT(GetRoot()->GetNext() == nullptr &&
                 GetRoot()->GetPosition() == 0 &&
                 dynamic_cast<RawItem*>(GetRoot()));

    auto root = static_cast<RawItem*>(GetRoot());
    auto hdr = HeaderItem::CreateAndInsert(this, root);
    auto exp = ExportsItem::CreateAndInsert(this, hdr);

    std::set<FilePosition> work, done;
    for (auto& et : exp->entries)
        work.insert(et.second->second.item->GetPosition() + et.second->second.offset);

    while (!work.empty())
    {
        auto pos = *work.begin();
        work.erase(work.begin());
        if (done.count(pos)) continue;

        auto instr = InstructionItem::CreateAndInsert(this, GetPointer(pos));
        done.insert(pos);

        if (instr->is_call)
            work.insert(ToFilePos(instr->target->second));
        if (instr->is_call || !no_returns.count(instr->opcode))
            work.insert(instr->GetNext()->GetPosition());

        for (const auto& p : instr->params)
        {
            if (p.type == InstructionItem::Param::INSTR_PTR0 ||
                p.type == InstructionItem::Param::INSTR_PTR1)
                work.insert(ToFilePos(p.param_4.label->second));
        }
    }
}

}
