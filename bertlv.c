#include <string.h>
#include "bertlv.h"

static const uint8_t tag_mask_first         = 0x1F;
static const uint8_t tag_mask_more          = 0x80;
static const uint8_t len_mask_long_format   = 0x80;

//------------------------------------------------------------------------------
//---- Tag ---------------------------------------------------------------------
//------------------------------------------------------------------------------
static
size_t bertlv_tag_calc_encode_size(bertlv_tag_t tag)
{
    size_t count;
    for(count = 0; tag; ++count, tag >>= 8)
    {}

    return count;
}
//------------------------------------------------------------------------------
static
size_t bertlv_tag_encode(void *buf, size_t bufsize, bertlv_tag_t tag)
{
    size_t tagsize = bertlv_tag_calc_encode_size(tag);
    if( !buf ) return tagsize;

    if( !tagsize || bufsize < tagsize ) return 0;

    for(uint8_t *pos = (uint8_t*)buf + tagsize - 1;
        tag;
        tag >>= 8)
    {
        *pos-- = tag & 0xFF;
    }

    return tagsize;
}
//------------------------------------------------------------------------------
static
size_t bertlv_tag_calc_decode_size(const uint8_t *data)
{
    if( !(*data) ) return 0;

    if( ( *data & tag_mask_first ) != tag_mask_first ) return 1;

    size_t count = 2;
    for(++data; *data & tag_mask_more; ++data, ++count)
    {}

    return count;
}
//------------------------------------------------------------------------------
static
size_t bertlv_tag_decode(const void *data, bertlv_tag_t *tag)
{
    const uint8_t *pos = data;
    size_t size = bertlv_tag_calc_decode_size(pos);
    if( !size ) return 0;

    *tag = 0;
    for(size_t i=0; i<size; ++i)
    {
        *tag <<= 8;
        *tag |= pos[i];
    }

    return size;
}
//------------------------------------------------------------------------------
static
size_t bertlv_tag_calc_num_encode_size(long num)
{
    if( num <= 0 ) return 0;

    size_t size;
    for(size=1; num; ++size, num >>= 7)
    {}

    return size;
}
//------------------------------------------------------------------------------
bertlv_tag_t bertlv_tag_make(int cla, int type, long num)
{
    /**
     * Make a tag value by combine the specific class, type and number.
     *
     * @param cla  The class value.
     * @param type The type value.
     * @param num  The number.
     * @return It returns a tag value be combined by the input information.
     *
     * @remarks This function will not check if the input values are valid,
     *          and the caller should be responsible for that.
     *          And the input properties may be truncated if they are out of range.
     */
    if( num < 0x1F )
        return ( ( cla & 0x03 ) << 6 ) | ( ( type & 0x01 ) << 5 ) | ( num & tag_mask_first );

    size_t size = bertlv_tag_calc_num_encode_size(num);
    uint8_t raw[size];

    raw[0] = ( ( cla & 0x03 ) << 6 ) | ( ( type & 0x01 ) << 5 ) | tag_mask_first;
    for(size_t i=1; i<size; ++i)
    {
        raw[i] = ( num >> 7*( size - i - 1 ) ) & 0x7F;
        if( i + 1 != size )
            raw[i] |= tag_mask_more;
    }

    bertlv_tag_t tag = 0;
    for(int i=0; i<size; ++i)
    {
        tag <<= 8;
        tag |= raw[i];
    }

    return tag;
}
//------------------------------------------------------------------------------
bool bertlv_tag_is_valid(bertlv_tag_t tag)
{
    /**
     * Check if a specific tag is a valid tag.
     */
    if( !tag ) return false;

    uint8_t raw[16];
    if( !bertlv_tag_encode(raw, sizeof(raw), tag) ) return false;

    bertlv_tag_t tag2;
    if( !bertlv_tag_decode(raw, &tag2) ) return false;

    return tag == tag2;
}
//------------------------------------------------------------------------------
int bertlv_tag_get_class(bertlv_tag_t tag)
{
    /**
     * Get the class value of a specific tag.
     */
    uint8_t raw[16];
    if( !bertlv_tag_encode(raw, sizeof(raw), tag) ) return 0;

    return ( raw[0] & 0xC0 ) >> 6;
}
//------------------------------------------------------------------------------
int bertlv_tag_get_type(bertlv_tag_t tag)
{
    /**
     * Get the type value of a specific tag.
     */
    uint8_t raw[16];
    if( !bertlv_tag_encode(raw, sizeof(raw), tag) ) return 0;

    return ( raw[0] & 0x20 ) >> 5;
}
//------------------------------------------------------------------------------
long bertlv_tag_get_number(bertlv_tag_t tag)
{
    /**
     * Get the number of a specific tag.
     */
    uint8_t raw[16];
    size_t size = bertlv_tag_encode(raw, sizeof(raw), tag);
    if( !size ) return 0;

    if( ( raw[0] & tag_mask_first ) != tag_mask_first )
        return raw[0] & tag_mask_first;

    long num = 0;
    for(size_t i=1; i<size; ++i)
    {
        num <<= 7;
        num |= raw[i] & ~tag_mask_more;
    }

    return num;
}
//------------------------------------------------------------------------------
//---- Length Field ------------------------------------------------------------
//------------------------------------------------------------------------------
static
size_t bertlv_len_calc_encode_size(size_t length)
{
    if( length <= 0x7F ) return 1;

    size_t count;
    for(count = 0; length; ++count, length >>= 8)
    {}

    return count + 1;
}
//------------------------------------------------------------------------------
static
size_t bertlv_len_encode(void *buf, size_t bufsize, size_t length)
{
    size_t lensize = bertlv_len_calc_encode_size(length);
    if( !buf ) return lensize;

    if( bufsize < lensize ) return 0;

    size_t subsequence_count = lensize - 1;
    if( subsequence_count )
    {
        uint8_t *pos = (uint8_t*)buf + subsequence_count;

        for(; length; length >>= 8)
            *pos-- = length & 0xFF;

        *pos = 0x80 | subsequence_count;
    }
    else
    {
        uint8_t *pos = buf;
        pos[0] = length;
    }

    return lensize;
}
//------------------------------------------------------------------------------
static
size_t bertlv_len_calc_decode_size(const uint8_t *data)
{
    if( !( *data & len_mask_long_format ) ) return 1;

    size_t subsequence_count = *data & ~len_mask_long_format;
    if( subsequence_count == 0 || subsequence_count == 0x7F ) return 0;  // Invalid value.

    return 1 + subsequence_count;
}
//------------------------------------------------------------------------------
static
size_t bertlv_len_decode(const void *data, size_t *length)
{
    const uint8_t *pos = data;
    size_t size = bertlv_len_calc_decode_size(pos);
    if( !size ) return 0;

    if( size == 1 )
    {
        *length = pos[0];
    }
    else
    {
        *length = 0;
        for(size_t i=1; i<size; ++i)
        {
            *length <<= 8;
            *length |= pos[i];
        }
    }

    return size;
}
//------------------------------------------------------------------------------
//---- TLV Element -------------------------------------------------------------
//------------------------------------------------------------------------------
size_t bertlv_encode(void *buf, size_t bufsize, bertlv_tag_t tag, const void *data, size_t size)
{
    /**
     * Encode TLV data.
     *
     * @param buf     A buffer to be filled by the encoded TLV data,
     *                and it can be NULL to calculate buffer size that be needed.
     * @param bufsize Size of the output buffer.
     * @param tag     Tag of the TLV element.
     * @param data    Payload data of the TLV element.
     * @param size    Payload size of the TLV element.
     * @return It returns the size of data be filled to the output buffer if succeed; or
     *         ZERO if the buffer is not large enough or the input information was invalid; or
     *         The minimum size of output buffer that will be needed if @a buf was NULL.
     *
     * @remarks This function will not check if the tag value is correct,
     *          and the caller should responsible for it
     *          (::bertlv_tag_is_valid can be used for that).
     */
    if( buf )
    {
        uint8_t *pos  = buf;
        size_t   rest = bufsize;

        size_t tag_size = bertlv_tag_encode(pos, rest, tag);
        if( !tag_size ) return 0;
        pos  += tag_size;
        rest -= tag_size;

        size_t len_size = bertlv_len_encode(pos, rest, size);
        if( !len_size ) return 0;
        pos  += len_size;
        rest -= len_size;

        if( rest < size ) return 0;
        memcpy(pos, data, size);

        return tag_size + len_size + size;
    }
    else
    {
        return bertlv_tag_encode(NULL, 0, tag) +
               bertlv_len_encode(NULL, 0, size) +
               size;
    }
}
//------------------------------------------------------------------------------
bertlv_tag_t bertlv_get_tag(const void *tlv)
{
    /**
     * Get the tag value of a specified TLV data.
     *
     * @param tlv The TLV data to be parsed.
     * @return The tag value of the TLV data if succeed; or
     *         ZERO if the TLV is NULL or have incorrect format.
     */
    bertlv_tag_t tag;
    return ( tlv && bertlv_tag_decode(tlv, &tag) )?( tag ):( 0 );
}
//------------------------------------------------------------------------------
size_t bertlv_get_length(const void *tlv)
{
    /**
     * Get the payload size of a specified TLV data.
     *
     * @param tlv The TLV data to be parsed.
     * @return The size (including zero) of payload data if succeed; or
     *         ZERO if the TLV is NULL or have incorrect format.
     */
    const uint8_t *pos = tlv;
    if( !pos ) return 0;

    size_t tag_size = bertlv_tag_calc_decode_size(pos);
    if( !tag_size ) return 0;
    pos += tag_size;

    size_t len_value;
    return bertlv_len_decode(pos, &len_value) ? len_value : 0;
}
//------------------------------------------------------------------------------
const void* bertlv_get_value(const void *tlv)
{
    /**
     * Get the payload data of a specified TLV data.
     *
     * @param tlv The TLV data to be parsed.
     * @return A pointer be pointed to the payload data
     *         (no matter if the TLV have payload or not) if succeed; or
     *         NULL if the TLV is NULL or have incorrect format.
     */
    const uint8_t *pos = tlv;
    if( !pos ) return NULL;

    size_t tag_size = bertlv_tag_calc_decode_size(pos);
    if( !tag_size ) return NULL;
    pos += tag_size;

    size_t len_size = bertlv_len_calc_decode_size(pos);
    if( !len_size ) return NULL;
    pos += len_size;

    return pos;
}
//------------------------------------------------------------------------------
size_t bertlv_get_total_size(const void *tlv)
{
    /**
     * Calculate size of raw data of a specific TLV data.
     *
     * @param tlv The TLV data to be parsed.
     * @return The total size of the TLV data if succeed; or
     *         ZERO if the TLV is NULL or have incorrect format.
     */
    const uint8_t *pos = tlv;
    if( !pos ) return 0;

    size_t tag_size = bertlv_tag_calc_decode_size(pos);
    if( !tag_size ) return 0;
    pos += tag_size;

    size_t len_value;
    size_t len_size = bertlv_len_decode(pos, &len_value);
    if( !len_size ) return 0;

    return tag_size + len_size + len_value;
}
//------------------------------------------------------------------------------
//---- TLV group ---------------------------------------------------------------
//------------------------------------------------------------------------------
const void* bertlv_iter_get_next(bertlv_iter_t *iter)
{
    /**
     * @memberof bertlv_iter_t
     * @brief Get the next TLV element.
     *
     * @param iter The iterator object.
     * @return The next TLV element if found; or
     *         NULL if no more elements.
     */
    if( !iter->pos ) return NULL;

    const void *tlv = NULL;

    do
    {
        static const size_t tlvsize_min = 2;
        if( iter->size < tlvsize_min ) break;

        size_t tlvsize = bertlv_get_total_size(iter->pos);
        if( !tlvsize || tlvsize > iter->size ) break;

        tlv = iter->pos;

        iter->pos  += tlvsize;
        iter->size -= tlvsize;
    } while(false);

    if( !tlv )
    {
        iter->pos  = NULL;
        iter->size = 0;
    }

    return tlv;
}
//------------------------------------------------------------------------------
unsigned bertlv_grp_count(const void *group, size_t size)
{
    /**
     * Count the number of TLV elements in a group of TLV data.
     *
     * @param group The set of raw data of TLV elements.
     * @param size  Size of the input data.
     * @return The number of TLV elements be counted in the group.
     */
    unsigned count = 0;

    bertlv_iter_t iter;
    bertlv_iter_init(&iter, group, size);
    while( bertlv_iter_get_next(&iter) )
        ++count;

    return count;
}
//------------------------------------------------------------------------------
const void* bertlv_grp_find(const void *group, size_t size, bertlv_tag_t tag)
{
    /**
     * Find the specific TLV element in a group of TLV data by tag.
     *
     * @param group The set of raw data of TLV elements.
     * @param size  Size of the input data.
     * @param tag   Tag of the specific TLV element.
     * @return The TLV element in the group if found; or
     *         NULL if not found.
     */
    bertlv_iter_t iter;
    bertlv_iter_init(&iter, group, size);
    for(const void *tlv; ( tlv = bertlv_iter_get_next(&iter) ); )
    {
        if( bertlv_get_tag(tlv) == tag )
            return tlv;
    }

    return NULL;
}
//------------------------------------------------------------------------------
size_t bertlv_grp_calc_total_size(const void *group, size_t size)
{
    /**
     * Calculate total size of available TLVs in group.
     *
     * @param group The set of raw data of TLV elements.
     * @param size  Size of the input data.
     * @return The total size of available TLVs.
     */
    size_t total_size = 0;

    bertlv_iter_t iter;
    bertlv_iter_init(&iter, group, size);
    for(const void *tlv; ( tlv = bertlv_iter_get_next(&iter) ); )
        total_size += bertlv_get_total_size(tlv);

    return total_size;
}
//------------------------------------------------------------------------------
