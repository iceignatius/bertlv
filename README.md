BER-TLV Parser and Encoder
==========================


This is a data processor of Basic Encoding Rules (BER) format of TLV data.


## Build

This library currently have no makefile, library, or something like that.
What a user need to do is just add the following file to your project and compile them:

* bertlv.h
* bertlv.c


## Document

Doxygen can be used to generate documents,
and please refer to them to use library in details.


## Examples

### Encode a TLV data

    bertlv_tag_t = 0x9F37;
    uint8_t value[4] = { 0x01, 0x35, 0x79 };

    uint8_t raw[64];
    size_t size = bertlv_encode(raw, sizeof(raw), tag, value, sizeof(value));
    // Now we have raw data of tag 9F37 in `raw` with size in `size`.

### Parse a TLV data

    uint8_t tlv[] = { 0x9F, 0x37, 0x03, 0x01, 0x35, 0x79 };

    printf("Tag: %lu\n", bertlv_get_tag(tlv));
    // Get 9F37.

    printf("Length: %zu\n", bertlv_get_length(tlv));
    // Get 3.

    printf("Value: %p\n", bertlv_get_value(tlv));
    // Get a point be pointed to { 0x01, 0x35, 0x79 }.

    printf("TLV total size: %zu\n", bertlv_get_total_size(tlv));
    // Get 6.

### Parse a set of TLV data

    uint8_t group[] =
    {
        0x9F, 0x37, 0x03, 0x01, 0x35, 0x79,
        0x9F, 0x21, 0x03, 0x12, 0x45, 0x53,
        0x9A, 0x03, 0x17, 0x01, 0x05
    };

    const uint8_t *tlv;

    bertlv_iter_t iter;
    bertlv_iter_init(&iter, group, sizeof(group));
    while(( tlv = bertlv_iter_get_next(&iter) ))
    {
        print_tlv_information(tlv);
    }
