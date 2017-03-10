/**
 * @file
 * @brief     BER-TLV parser and encoder.
 * @details   A data processor of Basic Encoding Rules (BER) format of TLV data.
 * @author    王文佑
 * @date      2017/01/05
 * @copyright ZLib Licence
 */
#ifndef _BERTLV_H_
#define _BERTLV_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Tag Properties
 * @{
 */

/**
 * Tag class.
 */
enum bertlv_tag_class_t
{
    BERTLV_CLASS_UNIVERSAL      = 0,    ///< The type is native to ASN.1.
    BERTLV_CLASS_APPLICATION    = 1,    ///< The type is only valid for one specific application.
    BERTLV_CLASS_CONTEXT        = 2,    ///< Meaning of this type depends on the context.
    BERTLV_CLASS_PRIVATE        = 3,    ///< Defined in private specifications.
};

/**
 * Tag type.
 */
enum bertlv_tag_type_t
{
    BERTLV_TYPE_PRIMITIVE   = 0,    ///< The contents octets directly encode the element value.
    BERTLV_TYPE_CONSTRUCTED = 1,    ///< The contents octets contain 0, 1, or more element encodings.
};

/**
 * @}
 */

/**
 * @name Tag
 * @{
 */

/**
 * Tag value, it will be an integral value.
 */
typedef unsigned long bertlv_tag_t;

bertlv_tag_t bertlv_tag_make(int cla, int type, long num);

bool bertlv_tag_is_valid(bertlv_tag_t tag);

int  bertlv_tag_get_class (bertlv_tag_t tag);
int  bertlv_tag_get_type  (bertlv_tag_t tag);
long bertlv_tag_get_number(bertlv_tag_t tag);

/**
 * @}
 */

/**
 * @name TLV Element
 * @{
 */

size_t bertlv_encode(void *buf, size_t bufsize, bertlv_tag_t tag, const void *data, size_t size);

bertlv_tag_t bertlv_get_tag       (const void *tlv);
size_t       bertlv_get_length    (const void *tlv);
const void*  bertlv_get_value     (const void *tlv);
size_t       bertlv_get_total_size(const void *tlv);

/**
 * @}
 */

/**
 * @class bertlv_iter_t
 * @brief TLV group iterator.
 */
typedef struct bertlv_iter_t
{
    const uint8_t *pos;
    size_t         size;
} bertlv_iter_t;

static inline
void bertlv_iter_init(bertlv_iter_t *iter, const void *group, size_t size)
{
    /**
     * @memberof bertlv_iter_t
     * @brief Constructor.
     *
     * @param iter  The iterator it self.
     * @param group A set of data of TLV elements.
     * @param size  Size of the input data.
     */
    iter->pos  = group;
    iter->size = size;
}

const void* bertlv_iter_get_next(bertlv_iter_t *iter);

/**
 * @name TLV Group
 * @{
 */

unsigned    bertlv_grp_count(const void *group, size_t size);
const void* bertlv_grp_find (const void *group, size_t size, bertlv_tag_t tag);

/**
 * @}
 */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
