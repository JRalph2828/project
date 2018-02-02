#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include "pages.h"
#ifdef WINDOWS
#define bool char
#define false 0
#define true 1
#endif

#define PAGESIZE 4096
#define LEAF_ORDER 32
#define INTERNAL_ORDER 249

// TYPES.

/* Type representing the record
 * to which a given key refers.
 * In a real B+ tree system, the
 * record would hold data (in a database)
 * or a file (in an operating system)
 * or some other information.
 * Users can rewrite this part of the code
 * to change the type and content
 * of the value field.
 */
typedef struct record {
    int value;
} record;

/* Type representing a node in thr B+ tree
 * This type is general enough to serve for both
 * the leaf and the internal node.
 * The heart of the node is the array
 * of keys and the array of corresponding
 * pointers.  The relation between keys
 * and pointers differs between leaves and
 * internal nodes.  In a leaf, the index
 * of each key equals the index of its corresponding
 * pointer, with a maximum of order - 1 key-pointer
 * pairs.  The last pointer points to the
 * leaf to the right (or NULL in the case
 * of the rightmost leaf).
 * In an internal node, the first pointer
 * refers to lower nodes with keys less than
 * the smallest key in the keys array.  Then,
 * with indices i starting at 0, the pointer
 * at i + 1 points to the subtree with keys
 * greater than or equal to the key in this
 * node at index i.
 * The num_keys field is used to keep
 * track of the number of valid keys.
 * In an internal node, the number of valid
 * pointers is always num_keys + 1.
 * In a leaf, the number of valid pointers
 * to data is always num_keys.  The
 * last leaf pointer points to the next leaf.
 */

typedef struct node {
    void ** pointers;
    int * keys;
    struct node * parent;
    bool is_leaf;
    int num_keys;
    struct node * next; // Used for queue.
} node;

// GLOBALS

/* The order determines the maximum and minimum
 * number of entries (keys and pointers) in any
 * node.  Every node has at most order - 1 keys and
 * at least (roughly speaking) half that number.
 * Every leaf has as many pointers to data as keys,
 * and every internal node has one more pointer
 * to a subtree than the number of keys.
 * This global variable is initialized to the
 * default value.
 */
extern int leaf_order;

extern int internal_order;

/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */
extern node * queue;

/* The user can toggle on and off the "verbose"
 * property, which causes the pointer addresses
 * to be printed out in hexadecimal notation
 * next to their corresponding keys.
 */
extern bool verbose_output;

// Header page
HeaderPage * headPage;

// File pointer
FILE *fp;

// Buffer for save string
char BUF[120];

// FUNCTION

void print_file();
void print_tree();

// Load page to memory
void load_page(FILE * fp, int64_t page_offset, Page * temp);

// Initailize page
void init_head_page();
void init_page(Page *temp);

// Free list function
void insert_into_free_page(uint64_t offset);
uint64_t is_free_page(void);

// Open DB
int open_db(char * pathname);

// Find
uint64_t find_leaf_page(int64_t key);
char * find(int64_t key);
int cut(int length);

// Insert
Page * make_new_page(void);
Page * make_leaf_page(void);
int get_left_index(uint64_t parent_offset, uint64_t left_offset);
void insert_into_leaf(int64_t key, char *value, uint64_t page_offset);
void insert_into_leaf_after_splitting(int64_t key, char *value, uint64_t old_offset);
void insert_into_internal(uint64_t parent_offset, uint64_t left_offset, int left_index, int64_t key, uint64_t right_offset);
void insert_int_internal_after_splitting(uint64_t old_offset, int left_index, int64_t key, uint64_t right_offset);
void insert_into_parent(uint64_t left_offset, int64_t key, uint64_t right_offset);
void insert_into_new_root(uint64_t left_offset, int64_t key, uint64_t right_offset);
void make_first_page(int64_t key, char *value);
int insert(int64_t key, char *value);

// Delete
int get_neighbor_index(uint64_t current_offset);
void remove_entry_from_page(int64_t key, uint64_t offset);
void adjust_root(uint64_t offset);
void coalesce_pages(uint64_t current_offset, uint64_t neighbor_offset, int neighbor_index, int64_t k_prime);
void redistribute_pages(uint64_t current_offset, uint64_t neighbor_offset, int neighbor_index, int k_prime_index, uint64_t k_prime);
void delete_entry(int64_t key, uint64_t offset);
int delete(int64_t key);


#endif /* __BPT_H__*/
