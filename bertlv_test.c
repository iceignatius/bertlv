#include <assert.h>
#include <string.h>
#include "bertlv.h"

//------------------------------------------------------------------------------
void test_tags(void)
{
    {
        static const int  cla  = 2;
        static const int  type = 1;
        static const long num  = 6;
        static const bertlv_tag_t tag = 0xA6;

        assert( tag == bertlv_tag_make(cla, type, num) );
        assert( bertlv_tag_is_valid(tag) );

        assert( cla  == bertlv_tag_get_class (tag) );
        assert( type == bertlv_tag_get_type  (tag) );
        assert( num  == bertlv_tag_get_number(tag) );
    }

    {
        static const int  cla  = 2;
        static const int  type = 1;
        static const long num  = 793;
        static const bertlv_tag_t tag = 0xBF8619;

        assert( tag == bertlv_tag_make(cla, type, num) );
        assert( bertlv_tag_is_valid(tag) );

        assert( cla  == bertlv_tag_get_class (tag) );
        assert( type == bertlv_tag_get_type  (tag) );
        assert( num  == bertlv_tag_get_number(tag) );
    }
}
//------------------------------------------------------------------------------
void test_tlv_elements(void)
{
    {
        static const bertlv_tag_t tag = 0xDF07;
        static const uint8_t tlv[] = { 0xDF,0x07, 0x00 };

        uint8_t buf[1024] = {0};
        assert( sizeof(tlv) == bertlv_encode(buf, sizeof(buf), tag, NULL, 0) );
        assert( 0 == memcmp(buf, tlv, sizeof(tlv)) );

        assert( sizeof(tlv) == bertlv_get_total_size(tlv) );
        assert( tag == bertlv_get_tag(tlv) );
        assert( 0 == bertlv_get_length(tlv) );
    }

    {
        static const bertlv_tag_t tag = 0xDF07;
        static const uint8_t data[] = { 0x13,0x57,0x24 };
        static const uint8_t tlv[] = { 0xDF,0x07, 0x03, 0x13,0x57,0x24 };

        uint8_t buf[1024] = {0};
        assert( sizeof(tlv) == bertlv_encode(buf, sizeof(buf), tag, data, sizeof(data)) );
        assert( 0 == memcmp(buf, tlv, sizeof(tlv)) );

        assert( sizeof(tlv) == bertlv_get_total_size(tlv) );
        assert( tag == bertlv_get_tag(tlv) );
        assert( sizeof(data) == bertlv_get_length(tlv) );
        assert( 0 == memcmp(bertlv_get_value(tlv), data, sizeof(data)) );
    }

    {
        static const bertlv_tag_t tag = 0xDF07;
        static const uint8_t data[200] = {0};
        static const uint8_t tlv[2+2+200] = { 0xDF,0x07, 0x81,0xC8 };

        uint8_t buf[1024] = {0};
        assert( sizeof(tlv) == bertlv_encode(buf, sizeof(buf), tag, data, sizeof(data)) );
        assert( 0 == memcmp(buf, tlv, sizeof(tlv)) );

        assert( sizeof(tlv) == bertlv_get_total_size(tlv) );
        assert( tag == bertlv_get_tag(tlv) );
        assert( sizeof(data) == bertlv_get_length(tlv) );
        assert( 0 == memcmp(bertlv_get_value(tlv), data, sizeof(data)) );
    }

    {
        static const bertlv_tag_t tag = 0xDF07;
        static const uint8_t data[500] = {0};
        static const uint8_t tlv[2+3+500] = { 0xDF,0x07, 0x82,0x01,0xF4 };

        uint8_t buf[1024] = {0};
        assert( sizeof(tlv) == bertlv_encode(buf, sizeof(buf), tag, data, sizeof(data)) );
        assert( 0 == memcmp(buf, tlv, sizeof(tlv)) );

        assert( sizeof(tlv) == bertlv_get_total_size(tlv) );
        assert( tag == bertlv_get_tag(tlv) );
        assert( sizeof(data) == bertlv_get_length(tlv) );
        assert( 0 == memcmp(bertlv_get_value(tlv), data, sizeof(data)) );
    }
}
//------------------------------------------------------------------------------
void test_tlv_group(void)
{
    static const uint8_t tlv1[] = { 0xC1, 0x02, 0x11,0x11 };
    static const uint8_t tlv2[] = { 0xC2, 0x02, 0x22,0x22 };
    static const uint8_t tlv3[] = { 0xC3, 0x02, 0x33,0x33 };
    static const uint8_t tlv4[] = { 0xC4, 0x02, 0x44,0x44 };
    static const uint8_t tlv5[] = { 0xC5, 0x02, 0x55,0x55 };

    static const uint8_t group[] =
    {
        0xC1, 0x02, 0x11,0x11,  // TLV 1
        0xC2, 0x02, 0x22,0x22,  // TLV 2
        0xC3, 0x02, 0x33,0x33,  // TLV 3
        0xC4, 0x02, 0x44,0x44,  // TLV 4
        0xC5, 0x02, 0x55,0x55,  // TLV 5
        0x00, 0x00              // Trailing space
    };

    {
        bertlv_iter_t iter;
        bertlv_iter_init(&iter, group, sizeof(group));

        const uint8_t *tlv;

        tlv = bertlv_iter_get_next(&iter);
        assert( tlv );
        assert( 0 == memcmp(tlv, tlv1, sizeof(tlv1)) );

        tlv = bertlv_iter_get_next(&iter);
        assert( tlv );
        assert( 0 == memcmp(tlv, tlv2, sizeof(tlv2)) );

        tlv = bertlv_iter_get_next(&iter);
        assert( tlv );
        assert( 0 == memcmp(tlv, tlv3, sizeof(tlv3)) );

        tlv = bertlv_iter_get_next(&iter);
        assert( tlv );
        assert( 0 == memcmp(tlv, tlv4, sizeof(tlv4)) );

        tlv = bertlv_iter_get_next(&iter);
        assert( tlv );
        assert( 0 == memcmp(tlv, tlv5, sizeof(tlv5)) );

        tlv = bertlv_iter_get_next(&iter);
        assert( !tlv );
    }

    {
        assert( 5 == bertlv_grp_count(group, sizeof(group)) );

        const uint8_t *tlv;

        tlv = bertlv_grp_find(group, sizeof(group), 0xC3);
        assert( tlv );
        assert( 0 == memcmp(tlv, tlv3, sizeof(tlv3)) );

        tlv = bertlv_grp_find(group, sizeof(group), 0xC7);
        assert( !tlv );
    }
}
//------------------------------------------------------------------------------
int main(void)
{
    test_tags();
    test_tlv_elements();
    test_tlv_group();

    return 0;
}
//------------------------------------------------------------------------------
