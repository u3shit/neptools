#ifndef STCL_GBNL_HPP
#define STCL_GBNL_HPP
#pragma once

#include "../raw_item.hpp"
#include <boost/endian/arithmetic.hpp>

namespace Stcm
{

struct GbnlMessageDescriptor
{
    boost::endian::little_uint32_t message_id;
    boost::endian::little_uint32_t lipsync_chara_id;
    boost::endian::little_uint32_t field_08;
    boost::endian::little_uint32_t field_0c;
    boost::endian::little_uint32_t field_10;
    boost::endian::little_uint32_t name_id;
    boost::endian::little_uint32_t field_18;
    boost::endian::little_uint32_t audio_id;
    boost::endian::little_uint32_t text_offset;

    bool IsValid(size_t chunk_size) const noexcept;
};
static_assert(sizeof(GbnlMessageDescriptor) == 0x24, "");

struct GbnlControlDescriptor
{
    boost::endian::little_uint16_t first;
    boost::endian::little_uint16_t second;
};
static_assert(sizeof(GbnlControlDescriptor) == 0x04, "");

struct GbnlFooter
{
    char magic[3];
    char version;

    boost::endian::little_uint32_t field_04;
    boost::endian::little_uint32_t field_08;
    boost::endian::little_uint32_t field_0c;
    boost::endian::little_uint32_t flags;
    boost::endian::little_uint32_t descr_offset;
    boost::endian::little_uint32_t count_msgs;
    boost::endian::little_uint32_t msg_descr_size;
    boost::endian::little_uint16_t count_control;
    boost::endian::little_uint16_t field_22;
    boost::endian::little_uint32_t offset_control;
    boost::endian::little_uint32_t field_28;
    boost::endian::little_uint32_t offset_msgs;
    boost::endian::little_uint32_t field_30;
    boost::endian::little_uint32_t field_34;
    boost::endian::little_uint32_t field_38;
    boost::endian::little_uint32_t field_3c;

    bool IsValid(size_t chunk_size) const noexcept;
};
static_assert(sizeof(GbnlFooter) == 0x40, "");

class GbnlItem : public Item
{
public:
    GbnlItem(Key k, Context* ctx, const Byte* data, size_t len);
    static GbnlItem* CreateAndInsert(RawItem* ritem);

    void Dump(std::ostream& os) const override;
    void PrettyPrint(std::ostream& os) const override;
    size_t GetSize() const noexcept override;

    void WriteTxt(std::ostream& os) const;
    void ReadTxt(std::istream& is);

    struct Message
    {
        uint32_t message_id, lipsync_chara_id, field_08, field_0c, field_10,
            field_18, name_id, audio_id;
        std::string text;
        size_t offset;
    };

    uint32_t field_28, field_30;
    std::vector<Message> messages;
    std::vector<std::pair<uint16_t, uint16_t>> control;

    void RecalcSize();
private:
    size_t msgs_size;
};

}

#endif
