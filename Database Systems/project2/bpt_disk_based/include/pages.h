#ifndef _PAGES_H_
#define _PAGES_H_
#include <stdint.h>
#include <stdio.h>


/* Header Page */
typedef struct HeaderPage_ {
    uint64_t free_page_offset;
    uint64_t root_page_offset;
    uint64_t num_pages;
    char dummy[4072];
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
    int dummy[26];
    uint64_t page_offset;
    union{
        Pair pair[31];
        InPair in_pair[248];
    }set;
} Page;

#endif
