#ifndef _PAGES_H_
#define _PAGES_H_
#include <stdint.h>
#include <stdio.h>


/* Header Page */
typedef struct HeaderPage_ {
    uint64_t free_page_offset;
    uint64_t root_page_offset;
    uint64_t num_pages;
    int64_t page_LSN;
    char dummy[4064];
} HeaderPage;

/* Record, key & value */
typedef struct Pair_ {
    int64_t key;
    char value[120];
} Pair;

/* Internal node, key & page offset */
typedef struct InPair_ {
    int64_t key;
    uint64_t page_offset;
} InPair;

/* Use for leaf page or 
 * internal page or free page
 * If leaf page, use pair
 * else internal page, use in_pair 
 */
typedef struct Page_ {
    uint64_t parent_or_free_page_offset;
    int is_leaf;
    int num_keys;
    int64_t padding;
    int64_t Page_LSN;
    int dummy[22];
    uint64_t page_offset;
    union{
        Pair pair[31];
        InPair in_pair[248];
    }set;
} Page;

/* The page for result
 * of natural join
 */
typedef struct Join_result_ {
    Pair pair[32];
} Join_result;

typedef struct Buffer_{
    union{
        Page page;
        HeaderPage head;
        Join_result join_result;
    }frame_kind;
    int table_id;
    uint64_t page_offset;
    int is_dirty;
    int pin_count;
    int ref_bit;
} Buffer;

/* Log record for recovery
 */
typedef struct Log_{
    int64_t LSN;
    int64_t prev_LSN;
    int type;
    int table_id;
    int page_number;
    int offset;
    Pair old;
    Pair new;
} Log;

#endif
