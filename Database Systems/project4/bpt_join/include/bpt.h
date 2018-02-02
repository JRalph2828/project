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
#define NUM_FILE 10

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

// Current frame for clock algorithm
extern int clk_hand;

// Buffer pool for buffer manager
Buffer *buf_pool;

// Size of buffer pool
int MAXBUFFER;

// Buffer for save string
char BUF[120];

// Mapping table for table_id
extern int table_id_map_trigger;

/* PRINT FUNCTION */
void print_buffer(void);
void print_file(int);
//void print_tree(int);

/* PAGE MANAGEMENT */
void load_page(int, uint64_t, int);
void write_page(int, uint64_t, int);
void wrtie_result(int);
void copy_header_page(HeaderPage *, HeaderPage *);
void copy_page(Page *, Page *);
void init_page(Page *);

/* FREE LIST MANAGEMENT */
void insert_into_free_page(int, uint64_t);
uint64_t is_free_page(int);

/* BUFFER MANAGEMENT */
int clock_request(int, uint64_t);
int get_page_from_buffer(int, uint64_t);
int get_header_idx(int table_id);
void push_page_to_buffer(int, int, uint64_t, Page*);
void push_header_to_buffer(int, int, uint64_t, HeaderPage*);
void push_join_result_to_buffer(Join_result*, int);
int init_db(int);

/* OPEN TABLE */
int open_table(char *);

/* CLOSE TABLE */
int close_table(int);
int shutdown_db(void);

/* JOIN */
int join_table(int, int, char*);
void sort_merge(int, int);

/* FIND */
int find_first_page(int);
uint64_t find_leaf_page(int, int64_t);
char * find(int, int64_t);
int cut(int);

/* INSERT */
int make_new_page(int);
int get_left_index(int, uint64_t, uint64_t);
void insert_into_leaf(int, int64_t, char *, uint64_t);
void insert_into_leaf_after_splitting(int, int64_t, char *, uint64_t);
void insert_into_internal(int, uint64_t, uint64_t, int, int64_t, uint64_t);
void insert_int_internal_after_splitting(int, uint64_t, int, int64_t, uint64_t);
void insert_into_parent(int, uint64_t, int64_t, uint64_t);
void insert_into_new_root(int, uint64_t, int64_t, uint64_t);
void make_first_page(int, int64_t, char *);
int insert(int, int64_t, char *);

/* DELETE */
int get_neighbor_index(int, uint64_t);
void remove_entry_from_page(int, int64_t, uint64_t);
void adjust_root(int, uint64_t);
void coalesce_pages(int, uint64_t, uint64_t, int, int64_t);
void redistribute_pages(int, uint64_t, uint64_t, int, int, uint64_t);
void delete_entry(int, int64_t, uint64_t);
int delete(int, int64_t);


#endif /* __BPT_H__*/
