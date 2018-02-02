#include "bpt.h"
#include "pages.h"

int leaf_order = LEAF_ORDER;
int internal_order = INTERNAL_ORDER;
int clk_hand = 0;
FILE *table_id_map[NUM_FILE];
int header_page_idx[NUM_FILE];
int table_id_map_trigger = 1;
int JOIN = 0;
FILE *join_fp;

/* PRINT FUNCTION */

/* Print out current buffer */
void print_buffer(void){
    printf("\n######################################################\n");
    for(int i = 0; i < MAXBUFFER; i++){
        if(buf_pool[i].table_id == 0)
            continue;
        printf("[%d]buffer\n",i);
        if(buf_pool[i].page_offset == 0){
            printf("\tframe: header\n");
            printf("\tfree_page_offset: %ld\n",buf_pool[i].frame_kind.head.free_page_offset);
            printf("\troot_page_offset: %ld\n",buf_pool[i].frame_kind.head.root_page_offset);
            printf("\tnum_pages : %ld\n",buf_pool[i].frame_kind.head.num_pages);
        }
        else{
            if(buf_pool[i].frame_kind.page.is_leaf)
                printf("\tframe: leaf page\n");
            else
                printf("\tframe: internal page\n");
            printf("\tparent_or_free_page_offset: %ld\n",buf_pool[i].frame_kind.page.parent_or_free_page_offset);
            printf("\tis_leaf: %d\n",buf_pool[i].frame_kind.page.is_leaf);
            printf("\tnum_keys: %d\n",buf_pool[i].frame_kind.page.num_keys);
            printf("\tpage_offset: %ld\n",buf_pool[i].frame_kind.page.page_offset);
            if(buf_pool[i].frame_kind.page.is_leaf){
                int j;
                for(j = 0; j < buf_pool[i].frame_kind.page.num_keys; j++){
                    printf("\t[%d]key & vlaue             : [%ld][%s]\n",j, buf_pool[i].frame_kind.page.set.pair[j].key, buf_pool[i].frame_kind.page.set.pair[j].value);
                }
            }
            else{
                int j;
                for(j = 0; j < buf_pool[i].frame_kind.page.num_keys; j++){
                    printf("\t[%d]key & page_offset       : [%ld][%ld]\n",j, buf_pool[i].frame_kind.page.set.in_pair[j].key, buf_pool[i].frame_kind.page.set.in_pair[j].page_offset);
                }
            }
        }
        printf("table_id: %d\n", buf_pool[i].table_id);
        printf("page_offset: %ld\n", buf_pool[i].page_offset);
        printf("is dirty: %d\n", buf_pool[i].is_dirty);
        printf("pin count: %d\n", buf_pool[i].pin_count);
        printf("ref bit: %d\n", buf_pool[i].ref_bit);
        printf("######################################################\n");
    }
    printf("\n");
    return;
}

/* Print out current file */
void print_file(int table_id){
    printf("######################################################\n");
    int size;
    FILE *fp;
    fp = table_id_map[table_id - 1];
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    printf("file size : %d\n", size);

    rewind(fp);

    HeaderPage *temp_head;
    temp_head = (HeaderPage*)calloc(1, PAGESIZE);
    fread(temp_head, 1, PAGESIZE, fp);

    printf("in Head \n");
    printf("\tfree_page_offset : %ld\n", temp_head->free_page_offset);
    printf("\troot_page_offset : %ld\n", temp_head->root_page_offset);
    printf("\tnum_pages        : %ld\n", temp_head->num_pages);

    Page *temp;
    temp = (Page*)calloc(1,PAGESIZE);

    while(1){
        init_page(temp);
        fread(temp, 1, PAGESIZE, fp);
        if(feof(fp)) break;
        printf("in page %ld\n", ftell(fp)-PAGESIZE);
        printf("\tparent_or_free_page_offset : %ld\n", temp->parent_or_free_page_offset);
        printf("\tis_leaf                    : %d\n", temp->is_leaf);
        printf("\tnum_keys                   : %d\n", temp->num_keys);
        printf("\tpage_offset                : %ld\n", temp->page_offset);
        if(temp->is_leaf){
            int i;
            for(i = 0; i < temp->num_keys; i++){
                printf("\t[%d]key & vlaue             : [%ld][%s]\n",i, temp->set.pair[i].key, temp->set.pair[i].value);
            }
        }
        else{
            int i;
            for(i = 0; i < temp->num_keys; i++){
                printf("\t[%d]key & page_offset       : [%ld][%ld]\n",i, temp->set.in_pair[i].key, temp->set.in_pair[i].page_offset);
            }
        }
    }
    free(temp_head);
    free(temp);
    printf("######################################################\n\n\n");
}

/* print out current tree */
/*
void print_tree(table_id) {
    Page page_;
    Page *page;
    page = &page_;
    uint64_t offset;
    int64_t height;
    int i = 0;
    int64_t cur_height = 0;
    int64_t q[100000];
    int64_t h[100000];
    int head = 0;
    int tail = 0;

    q[tail] = headPage->root_page_offset;
 
    if(q[head] == 0)
        return;

    h[tail] = 0;
    tail++;
    while (head < tail) {
        offset = q[head];
        height = h[head];
        head++;

        if (height != cur_height) {
            fprintf(stderr, "\n");
            cur_height = height;
        }

        load_page(fp, offset, page);
        
        if (page->is_leaf) {
            for (i = 0; i < page->num_keys; i++) {
                if ( i != 0 )
                    fprintf(stderr, " | ");
                fprintf(stderr, "%" PRId64, page->set.pair[i].key);
            }
        } else {
            q[tail] = page->page_offset;
            h[tail] = height + 1;
            tail++;
            fprintf(stderr, "(%"PRIx64 ") ", page->page_offset);
            for (i = 0; i < page->num_keys; i++) {
                if ( i != 0 )
                    fprintf(stderr, " | ");
                fprintf(stderr, "%" PRId64 " (%"PRIx64 ")", page->set.in_pair[i].key, page->set.in_pair[i].page_offset);
                q[tail] = page->set.in_pair[i].page_offset;
                h[tail] = height + 1;
                tail++;
            }
        }

        fprintf (stderr, " ||| ");
    }
    fprintf(stderr, "\n\n");

}
*/

/* PAGE MANAGEMENT */

/* Read page from disk and
 * load page to buffer
 */
void load_page(int table_id, uint64_t page_offset, int current){
    fseek(table_id_map[table_id - 1], page_offset, SEEK_SET);
   
    /* Case: general page */ 
    if(page_offset != 0)
        fread(&buf_pool[current].frame_kind.page, 1, PAGESIZE, table_id_map[table_id - 1]);

    /* Case: header page */
    else
        fread(&buf_pool[current].frame_kind.head, 1, PAGESIZE, table_id_map[table_id - 1]);

    return;
}

/* Write buffer page to disk
 */
void write_page(int table_id, uint64_t page_offset, int current){
    fseek(table_id_map[table_id - 1], page_offset, SEEK_SET);
    
    /* Case: general page */
    if(page_offset != 0)
        fwrite(&buf_pool[current].frame_kind.page, 1, PAGESIZE, table_id_map[table_id - 1]);

    /* Case: header page */
    else
        fwrite(&buf_pool[current].frame_kind.head, 1, PAGESIZE, table_id_map[table_id - 1]);

    fflush(table_id_map[table_id - 1]);
    return;
}

void write_result(int result_num){

    for(int i = 0; i < result_num; i++){
        fprintf(join_fp, "%ld,%s,%ld,%s\n", 
                buf_pool[MAXBUFFER - 1].frame_kind.join_result.pair[2*i].key,
                buf_pool[MAXBUFFER - 1].frame_kind.join_result.pair[2*i].value,
                buf_pool[MAXBUFFER - 1].frame_kind.join_result.pair[2*i + 1].key,
                buf_pool[MAXBUFFER - 1].frame_kind.join_result.pair[2*i + 1].value);
    }

    fflush(join_fp);
    return;
}

/* Copy header page from 
 * buffer to local call stack
 */
void copy_header_page(HeaderPage *old, HeaderPage *new){
    memcpy(new, old, PAGESIZE);
    return;
}

/* Copy page from
 * buffer to local call stack
 */
void copy_page(Page *old, Page *new){
    memcpy(new, old, PAGESIZE);
    return;
}

/* Initialize page */
void init_page(Page *temp){
    memset(temp, 0, PAGESIZE);
    return;
}

/* FREE LIST MANAGEMENT */

/* Insert free page into free list */
void insert_into_free_page(int table_id, uint64_t offset){

    HeaderPage head;
    Page new;
    int idx;
    idx = get_page_from_buffer(table_id, offset);
    copy_page(&buf_pool[idx].frame_kind.page, &new);
    copy_header_page(&buf_pool[get_header_idx(table_id)].frame_kind.head, &head);

    
    /* Case: first free page */
    if(head.free_page_offset == 0){
        head.free_page_offset = offset;
        new.parent_or_free_page_offset = 0;
    }

    /* Case: insert new free page 
     * into free page list
     * (Rest of the function body)
     */
    else{
        new.parent_or_free_page_offset = head.free_page_offset;
        head.free_page_offset = offset;
    }

    push_header_to_buffer(table_id, get_header_idx(table_id), 0, &head);
    push_page_to_buffer(table_id, idx, offset, &new);

    return;
}

/* Look ahead header page's free page offset
 * If free page existing, use for page 
 * and return free page's index in buffer
 * else return
 */
uint64_t is_free_page(int table_id){

    /* Case: empty free page list */
    if(buf_pool[get_header_idx(table_id)].frame_kind.head.free_page_offset == 0)
        return 0;

    /* Case: free page exists
     * (Rest of the function body)
     */
    else{
        uint64_t offset;
        int idx;
        Page temp;
        HeaderPage head;

        copy_header_page(&buf_pool[get_header_idx(table_id)].frame_kind.head, &head);
        offset = head.free_page_offset;
        
        idx = get_page_from_buffer(table_id, offset);
        copy_page(&buf_pool[idx].frame_kind.page, &temp);

        head.free_page_offset = temp.parent_or_free_page_offset;

        push_header_to_buffer(table_id, get_header_idx(table_id), 0, &head);
        push_page_to_buffer(table_id, idx, offset, &temp);

        return offset;
    }
}

/* BUFFER MANAGEMENT */

/* Find possible page to use
 * by clock algorithm and
 * load new page from disk to buffer pool
 */
int clock_request(int table_id, uint64_t offset){
    int current = clk_hand;

    while(1){
        if(buf_pool[current].pin_count == 0 && buf_pool[current].ref_bit == 0){
            if(buf_pool[current].is_dirty == 1)
                write_page(buf_pool[current].table_id, buf_pool[current].page_offset, current);
           
            memset(&buf_pool[current].frame_kind.page, 0, PAGESIZE);
            load_page(table_id, offset, current);
            buf_pool[current].table_id = table_id;
            buf_pool[current].page_offset = offset;
            buf_pool[current].is_dirty = 0;
            buf_pool[current].pin_count = 1;
            buf_pool[current].ref_bit = 1;
            clk_hand = current;
            break;
        }
        else if(buf_pool[current].pin_count == 0 && buf_pool[current].ref_bit == 1)
            buf_pool[current].ref_bit = 0;

        current = (current + 1) % MAXBUFFER;
    }
    return current;
}

/* Get page index from buffer
 * If page not exists,
 * change frame by clock algorithm
 */
int get_page_from_buffer(int table_id, uint64_t offset){

     /* Case: page is in buffer pool
     */
    for(int i = 0; i < MAXBUFFER; i++){
        if(buf_pool[i].table_id == table_id &&
                buf_pool[i].page_offset == offset){
            buf_pool[i].pin_count = 1;
            buf_pool[i].ref_bit = 1;
            return i;
        }
    }

    /* Case: the page doesn't
     * exist in buffer pool
     */ 
    return clock_request(table_id, offset);
}

/* Get header frame index in
 * header_page_idx array
 */
int get_header_idx(int table_id){
    
    /* If join query is coming
     * modify header_page to buffer pool
     * mapping table
     */
    if(JOIN){
        int header_idx;
        header_idx = get_page_from_buffer(table_id, 0);
        header_page_idx[table_id - 1] = header_idx;
    }
        
    return header_page_idx[table_id - 1];
}

/* Find frame in buffer
 * and write data
 */
void push_page_to_buffer(int table_id, int target, uint64_t page_offset, Page* current){

    memcpy(&buf_pool[target].frame_kind.page, current, PAGESIZE);
    buf_pool[target].table_id = table_id;
    buf_pool[target].page_offset = page_offset;
    buf_pool[target].is_dirty = 1;
    buf_pool[target].pin_count = 0;

    return;
}

/* Find header frame in
 * buffer and write data
 */
void push_header_to_buffer(int table_id, int target, uint64_t page_offset, HeaderPage* head){

    memcpy(&buf_pool[target].frame_kind.head, head, PAGESIZE);
    buf_pool[target].table_id = table_id;
    buf_pool[target].page_offset = page_offset;
    buf_pool[target].is_dirty = 1;

    /* Header page will always be
     * in the buffer pool
     */
    buf_pool[target].pin_count = 1;

    return ;
}

void push_join_result_to_buffer(Join_result* result, int num_result){
    memcpy(&buf_pool[MAXBUFFER - 1].frame_kind.join_result, result, PAGESIZE);
    buf_pool[MAXBUFFER - 1].pin_count = 1;
    
    write_result(num_result);
    return;
}

/* Create buffer pool
 * with given number
 * and initialize buffer
 */
int init_db(int num_buf){
    MAXBUFFER = num_buf;
    buf_pool = (Buffer*)calloc(MAXBUFFER, sizeof(Buffer));

    if(buf_pool == NULL)
        return -1;
    else{
        for(int i = 0; i < MAXBUFFER; i++)
            buf_pool[i].page_offset = -1;
        return 0;
    }
}


/* OPEN TABLE */

/* Open existing data file using
 * ‘pathname’ or create one
 * if not existed
 */
int open_table(char *pathname){
    FILE *fp;
    fp = NULL;

    fp = fopen(pathname, "r+b");

    HeaderPage head;
    int header_page_index;

    /* Initialize mapping table
     * when first time open_table
     * is called
     */
    if(table_id_map_trigger){
        for(int i = 0; i < NUM_FILE; i++){
            table_id_map[i] = NULL;
            header_page_idx[i] = -1;
        }
        table_id_map_trigger = 0;
    }

    /* Create data file and
     * initialize header file
     * and return table_id */
    if(fp == NULL){
        fp = fopen(pathname, "w+b");
        
        for(int i = 0; i < NUM_FILE; i++){
            if(table_id_map[i] == NULL){
                table_id_map[i] = fp;
                header_page_index = make_new_page(i + 1);
                head.num_pages = 1;
                push_header_to_buffer(i + 1, header_page_index, 0, &head);
                header_page_idx[i] = header_page_index;
                return i + 1;
            }
        }
    }

    /* Open existing data file and
     * read header page from file
     * and return table_id */
    else{
        for(int i =0; i < NUM_FILE; i++){
            if(table_id_map[i] == NULL){
                table_id_map[i] = fp;
                header_page_index = get_page_from_buffer(i + 1, 0);
                header_page_idx[i] = header_page_index;
                return i + 1;
            }
        }
    }

    /* If fail, return -1 */
    return -1;
}

/* CLOSE TABLE */

/* Write all pages of this table 
 * from buffer to disk 
 * and discard the table id
 */
int close_table(int table_id){

    for(int i = 0; i < MAXBUFFER; i++){
        if(buf_pool[i].table_id == table_id && buf_pool[i].is_dirty == 1){
            write_page(table_id, buf_pool[i].page_offset, i);
            memset(&buf_pool[i], 0, sizeof(Buffer));
            buf_pool[i].table_id = -1; 
        }
    }

    if(fclose(table_id_map[table_id - 1]) != 0){
        //table_id_map[table_id - 1] = NULL;
        //header_page_idx[table_id - 1] = -1;
        return -1;
    }
    else{
        table_id_map[table_id - 1] = NULL;
        header_page_idx[table_id - 1] = -1;
        return 0;
    }
}

/* Flush all data from buffer 
 * and destroy allocated buffer
 */
int shutdown_db(void){
    for(int i = 0; i < MAXBUFFER; i++){
        if(buf_pool[i].table_id != -1){
            if(buf_pool[i].is_dirty == 1)
                write_page(buf_pool[i].table_id, buf_pool[i].page_offset, i);
            if(buf_pool[i].page_offset == 0)
                write_page(buf_pool[i].table_id, buf_pool[i].page_offset, i);
        }
    }

    for(int i = 0; i < NUM_FILE; i++)
        if(table_id_map[i] != NULL)
            if(fclose(table_id_map[i]) != 0)
                return -1;
    
    free(buf_pool);
    
    return 0;
}

/* JOIN */

/* Do natural join with given 
 * two tables and write result table 
 * to the file using given pathname.
 */
int join_table(int table_id_1, int table_id_2, char *pathname){
    
    int header_idx;
    /* Set flag for
     * join query 
     * */
    JOIN = 1;

    join_fp = NULL;
    join_fp = fopen(pathname, "w+t");
    
    if(join_fp == NULL)
        return -1;
    
    for(int i = 0; i < MAXBUFFER; i++){
        if(buf_pool[i].page_offset == 0)
            write_page(buf_pool[i].table_id, buf_pool[i].page_offset, i);
    }

    /* Initialize 
     * buffer pool
     */
    for(int i = 0; i < MAXBUFFER; i++)
        memset(&buf_pool[i], 0, sizeof(Buffer));

    /* Last buffer will
     * be used for 
     * output buffer
     */
    buf_pool[MAXBUFFER - 1].pin_count = 1;
    buf_pool[MAXBUFFER - 1].ref_bit = 1;
    buf_pool[MAXBUFFER - 1].page_offset = -1;

    sort_merge(table_id_1, table_id_2);

    JOIN = 0;
        
    header_idx = get_page_from_buffer(table_id_1, 0);
    header_page_idx[table_id_1 - 1] = header_idx;
    header_idx = get_page_from_buffer(table_id_2, 0);
    header_page_idx[table_id_2 - 1] = header_idx;

    fclose(join_fp);

    return 0;
}

/* Merge for two tables
 * that already sorted
 */
void sort_merge(int table_id_1, int table_id_2){
    bool is_done = false;
    int i = 0;
    int j = 0;
    int k = 0;
    int idx_1;
    int idx_2;
    Page page_1;
    Page page_2;
    Join_result result;

    /* Find page that
     * includes first key
     */
    idx_1 = find_first_page(table_id_1);
    copy_page(&buf_pool[idx_1].frame_kind.page, &page_1);
    idx_2 = find_first_page(table_id_2);
    copy_page(&buf_pool[idx_2].frame_kind.page, &page_2);

    /* Merge two tables
     * as natural join
     */
    while(!is_done){
        while(page_1.set.pair[i].key < page_2.set.pair[j].key){
            
            i++;
            if(i == page_1.num_keys){
                if(page_1.page_offset == 0){
                    buf_pool[idx_1].pin_count = 0;
                    buf_pool[idx_1].ref_bit = 0;
                    buf_pool[idx_2].pin_count = 0;
                    buf_pool[idx_2].ref_bit = 0;
                    push_join_result_to_buffer(&result, k);
                    is_done = true;
                }
                else{
                    buf_pool[idx_1].pin_count = 0;
                    buf_pool[idx_1].ref_bit = 0;
                    idx_1 = get_page_from_buffer(table_id_1, page_1.page_offset);
                    copy_page(&buf_pool[idx_1].frame_kind.page, &page_1);
                    i = 0;
                }
            }
        }

        if(is_done)
            break;

        while(page_1.set.pair[i].key > page_2.set.pair[j].key){
        
            j++;
            if(j == page_2.num_keys){
                if(page_2.page_offset == 0){
                    buf_pool[idx_1].pin_count = 0;
                    buf_pool[idx_1].ref_bit = 0;
                    buf_pool[idx_2].pin_count = 0;
                    buf_pool[idx_2].ref_bit = 0;
                    push_join_result_to_buffer(&result, k);
                    is_done = true;
                }
                else{
                    buf_pool[idx_2].pin_count = 0;
                    buf_pool[idx_2].ref_bit = 0;
                    idx_2 = get_page_from_buffer(table_id_2, page_2.page_offset);
                    copy_page(&buf_pool[idx_2].frame_kind.page, &page_2);
                    j = 0;
                }
            }
        }

        if(is_done)
            break;

        /* If output buffer is full
         * write to disk
         */
        if(k == 16){
            push_join_result_to_buffer(&result, k);
            k = 0;
        }

        if(page_1.set.pair[i].key == page_2.set.pair[j].key){
            result.pair[2*k].key = page_1.set.pair[i].key;
            strcpy(result.pair[2*k].value, page_1.set.pair[i].value);
            result.pair[2*k + 1].key = page_2.set.pair[j].key;
            strcpy(result.pair[2*k + 1].value, page_2.set.pair[j].value);
            k++;
            j++;
        }

        /* If index j is over 
         * than page keys 
         * get next page
         */
        if(j == page_2.num_keys){
            if(page_2.page_offset == 0){
                buf_pool[idx_1].pin_count = 0;
                buf_pool[idx_1].ref_bit = 0;
                buf_pool[idx_2].pin_count = 0;
                buf_pool[idx_2].ref_bit = 0;
                push_join_result_to_buffer(&result, k);
                is_done = true;
            }
            else{
                buf_pool[idx_2].pin_count = 0;
                buf_pool[idx_2].ref_bit = 0;
                idx_2 = get_page_from_buffer(table_id_2, page_2.page_offset);
                copy_page(&buf_pool[idx_2].frame_kind.page, &page_2);
                j = 0;
            }
        }
    }

    return ;
}

/* FIND */

/* Find first key page */
int find_first_page(int table_id){
    HeaderPage head;
    Page temp;
    uint64_t offset;
    int idx;
    int head_idx;

    head_idx = get_header_idx(table_id);
    copy_header_page(&buf_pool[head_idx].frame_kind.head, &head);
    buf_pool[head_idx].pin_count = 0;
    buf_pool[head_idx].ref_bit = 0;

    offset = head.root_page_offset;
    idx = get_page_from_buffer(table_id, offset);
    copy_page(&buf_pool[idx].frame_kind.page, &temp);

    /* Case: first key
     * is in the root page
     */
    if(temp.is_leaf){
        return idx;
    }

    /* Case: first key
     * is not in root page
     * (rest of function body)
     */
    else{
        while(!temp.is_leaf){
            buf_pool[idx].pin_count =0;
            buf_pool[idx].ref_bit = 0;
            offset = temp.page_offset;
            idx = get_page_from_buffer(table_id, offset);
            copy_page(&buf_pool[idx].frame_kind.page, &temp);
        }
        return idx;
    }
}

/* Find leaf page */
uint64_t find_leaf_page(int table_id, int64_t key){

    int i = 0;
    int idx;
    uint64_t offset;
    HeaderPage head;
    Page temp;
    
    copy_header_page(&buf_pool[get_header_idx(table_id)].frame_kind.head, &head);

    // Get root page offset
    uint64_t root_page_offset = head.root_page_offset;

    // Load root page to buffer
    idx = get_page_from_buffer(table_id, root_page_offset);
    copy_page(&buf_pool[idx].frame_kind.page, &temp);

    offset = root_page_offset;

    buf_pool[idx].pin_count =0;
    
    if(head.root_page_offset == 0)
       return offset;

    // Find until meet leaf page
    while(!temp.is_leaf){
        i = 0;
        if(key < temp.set.in_pair[0].key){
            offset = temp.page_offset;
            idx = get_page_from_buffer(table_id, offset);
            copy_page(&buf_pool[idx].frame_kind.page, &temp);
            buf_pool[idx].pin_count =0;
        }
        else{
            while(i < temp.num_keys){
                if(key >= temp.set.in_pair[i].key) i++;
                else break;
            }
            offset = temp.set.in_pair[i-1].page_offset;
            idx = get_page_from_buffer(table_id, offset);
            copy_page(&buf_pool[idx].frame_kind.page, &temp);
            buf_pool[idx].pin_count =0;
        } 
    }
    return offset;
}

/* Find the record containing input 'key' */
char * find(int table_id, int64_t key){
    
    // Allocate temp page
    Page temp;
    int idx;
    uint64_t offset;

    // Find leaf page
    offset = find_leaf_page(table_id, key);
    idx = get_page_from_buffer(table_id, offset);
    copy_page(&buf_pool[idx].frame_kind.page, &temp);
    
    // Check for existing key
    int i = 0;
    for(i = 0; i < temp.num_keys; i++)
        if(temp.set.pair[i].key == key)
            break;

    // Fail for finding key
    if(i == temp.num_keys){
        buf_pool[idx].pin_count = 0;
        //buf_pool[idx].ref_bit = 0;
        return NULL;
    }

    // Success for finding key
    else{
        buf_pool[idx].pin_count = 0;
        memset(BUF, 0, 120);
        strcpy(BUF, temp.set.pair[i].value);
        return BUF;
    }
}

/* Find the appropriate place to
 * split a page that is too big into two
 */
int cut(int length) {
    if(length % 2 == 0)
        return length/2;
    else
        return length/2 +1;
}

/* Insert */

/* Creates a new frame to buffer for 
 * general page, which can be adapted
 * to serve as either a leaf or an internal page
 * by clock algorithm
 */
int make_new_page(int table_id){
    int current = clk_hand;

    while(1){
        if(buf_pool[current].pin_count == 0 && buf_pool[current].ref_bit == 0){
            if(buf_pool[current].is_dirty == 1)
                write_page(buf_pool[current].table_id, buf_pool[current].page_offset, current);

            memset(&buf_pool[current].frame_kind.page, 0, PAGESIZE);
            buf_pool[current].page_offset = -1;
            buf_pool[current].table_id = table_id;
            buf_pool[current].is_dirty = 0;
            buf_pool[current].pin_count = 1;
            buf_pool[current].ref_bit = 1;
            clk_hand = current;
            break;
        }
        else if(buf_pool[current].pin_count == 0 && buf_pool[current].ref_bit == 1)
            buf_pool[current].ref_bit = 0;

        current = (current + 1) % MAXBUFFER;

    }
    return current;
}

/* Helper function used in insert_into_parent
 * ro find the index of the parent's offset to
 * the page to the left of the key to be inserted
 */
int get_left_index(int table_id, uint64_t parent_offset, uint64_t left_offset){

    Page parent;
    int parent_idx;
    parent_idx = get_page_from_buffer(table_id, parent_offset);
    copy_page(&buf_pool[parent_idx].frame_kind.page, &parent);

    Page left;
    int left_idx;
    left_idx = get_page_from_buffer(table_id, left_offset);
    copy_page(&buf_pool[left_idx].frame_kind.page, &left);

    int left_index = 0;

    if(left_offset == parent.page_offset)
        return -1;
    else{
        while(left_index <= parent.num_keys &&
            parent.set.in_pair[left_index].page_offset != left_offset)
            left_index++;
        
        left_index++;
        
        return left_index;
    }
}

/* Inserts a new key and value to leaf page*/
void insert_into_leaf(int table_id, int64_t key, char *value, uint64_t page_offset){
    int i, insertion_point;

    int idx;
    Page temp;
    idx = get_page_from_buffer(table_id, page_offset);
    copy_page(&buf_pool[idx].frame_kind.page, &temp);

    insertion_point = 0;

    while(insertion_point < temp.num_keys && 
            temp.set.pair[insertion_point].key < key)
        insertion_point++;

    for(i = temp.num_keys; i > insertion_point; i--){
        temp.set.pair[i].key = temp.set.pair[i-1].key;
        strcpy(temp.set.pair[i].value, temp.set.pair[i-1].value);
    }
    temp.set.pair[insertion_point].key = key;
    strcpy(temp.set.pair[insertion_point].value, value);
    temp.num_keys++;

    push_page_to_buffer(table_id, idx, page_offset, &temp);
    
    return;
}

/* Insert a new key and value
 * to a new leaf page so as to exceed
 * the tree's order, causing the leaf to be split
 * in half
 */
void insert_into_leaf_after_splitting(int table_id, int64_t key, char *value, uint64_t old_offset){
    HeaderPage head;
    Page old_leaf;
    Page new_leaf;
    Pair *temp_pairs;
    int insertion_index, split, i, j;
    uint64_t old_parent_page_offset;
    int old_is_leaf;
    uint64_t old_page_offset;
    uint64_t new_offset;
    int64_t new_key;
    int old_idx;
    int new_idx;

    copy_header_page(&buf_pool[get_header_idx(table_id)].frame_kind.head, &head);
    old_idx = get_page_from_buffer(table_id, old_offset);
    copy_page(&buf_pool[old_idx].frame_kind.page, &old_leaf);
    
    old_parent_page_offset = old_leaf.parent_or_free_page_offset;
    old_is_leaf = old_leaf.is_leaf;
    old_page_offset = old_leaf.page_offset;

    init_page(&new_leaf);

    temp_pairs = (Pair*)calloc(leaf_order, sizeof(Pair));

    insertion_index = 0;

    while(insertion_index < leaf_order - 1 && old_leaf.set.pair[insertion_index].key < key)
        insertion_index++;

    for(i = 0, j = 0; i < old_leaf.num_keys; i++, j++){
        if(j == insertion_index) j++;
        temp_pairs[j].key = old_leaf.set.pair[i].key;
        strcpy(temp_pairs[j].value, old_leaf.set.pair[i].value);
    }

    temp_pairs[insertion_index].key = key;
    strcpy(temp_pairs[insertion_index].value, value);

    init_page(&old_leaf);
    old_leaf.parent_or_free_page_offset = old_parent_page_offset;
    old_leaf.is_leaf = old_is_leaf; 
    old_leaf.page_offset = old_page_offset;

    old_leaf.num_keys = 0;

    split = cut(leaf_order - 1);

    for(i = 0; i < split; i++){
        old_leaf.set.pair[i].key = temp_pairs[i].key;
        strcpy(old_leaf.set.pair[i].value, temp_pairs[i].value);
        old_leaf.num_keys++;
    }

    for(i = split, j= 0; i < leaf_order; i++, j++){
        new_leaf.set.pair[j].key = temp_pairs[i].key;
        strcpy(new_leaf.set.pair[j].value, temp_pairs[i].value);
        new_leaf.num_keys++;
    }

    free(temp_pairs);

    new_leaf.page_offset = old_leaf.page_offset;
    new_leaf.is_leaf = true;

    /* Check if there is free page */

    new_offset = is_free_page(table_id);

    // If there is a free page?
    if(new_offset != 0){
        old_leaf.page_offset = new_offset;
        new_idx = get_page_from_buffer(table_id, new_offset);
    }

    // Else no free page
    else{
        head.num_pages++;
        push_header_to_buffer(table_id, get_header_idx(table_id), 0, &head);
        old_leaf.page_offset = (head.num_pages - 1)*PAGESIZE;
        new_idx = make_new_page(table_id);
    }

    new_leaf.parent_or_free_page_offset = old_leaf.parent_or_free_page_offset;
    new_key = new_leaf.set.pair[0].key;

    push_page_to_buffer(table_id, old_idx, old_offset, &old_leaf);
    push_page_to_buffer(table_id, new_idx, old_leaf.page_offset, &new_leaf);

    new_offset = old_leaf.page_offset;
    insert_into_parent(table_id, old_offset, new_key, new_offset);

    return;
}

/* Inserts a new key and offset to a page
 * into a page into which these can fit
 * without violating the B+ tree properties
 */
void insert_into_internal(int table_id, uint64_t parent_offset, uint64_t left_offset,
        int left_index, int64_t key, uint64_t right_offset) {
   
    Page temp;
    int idx;
    idx = get_page_from_buffer(table_id, parent_offset); 
    copy_page(&buf_pool[idx].frame_kind.page, &temp);

    int i;
    if(left_index == -1){
        for(i = temp.num_keys; i > 0; i--){
            temp.set.in_pair[i].page_offset = temp.set.in_pair[i-1].page_offset;
            temp.set.in_pair[i].key = temp.set.in_pair[i-1].key;
        }

        temp.page_offset = left_offset;
        temp.set.in_pair[0].key = key;
        temp.set.in_pair[0].page_offset = right_offset;
    } 

    else{
        for(i = temp.num_keys; i > left_index; i--){
            temp.set.in_pair[i].page_offset = temp.set.in_pair[i-1].page_offset;
            temp.set.in_pair[i].key = temp.set.in_pair[i-1].key;
        }
        temp.set.in_pair[left_index].page_offset = right_offset;
        temp.set.in_pair[left_index].key = key;
    }

    temp.is_leaf = false;
    temp.num_keys++;

    push_page_to_buffer(table_id, idx, parent_offset, &temp);

    return;
}

/* Inserts a new key and offset to a page
 * into a page, causing the pages's size to exceed
 * the order, and causing the page to split into two
 */
void insert_into_internal_after_splitting(int table_id, uint64_t old_offset, 
        int left_index, int64_t key, uint64_t right_offset){

    int i, j, split; 
    int64_t k_prime;
    uint64_t new_offset, old_parent_page_offset, old_page_offset;
    int old_is_leaf;
    int new_idx;
    int child_idx;
    int old_idx;
    HeaderPage head;
    Page new_page;
    Page child_page;
    Page old_page;
    InPair * temp_in_pairs;

    /* First create a temporary set of keys and offsets
     * to hold everything in order, including
     * the new key and offset, inserted in their
     * correct places
     * Then create a new page and copy half of the
     * keys and offsets to the old page and
     * the other half to the new
     */

    copy_header_page(&buf_pool[get_header_idx(table_id)].frame_kind.head, &head);
    temp_in_pairs = (InPair*)calloc(internal_order, sizeof(InPair));

    old_idx = get_page_from_buffer(table_id, old_offset);
    copy_page(&buf_pool[old_idx].frame_kind.page, &old_page);
    
    old_parent_page_offset = old_page.parent_or_free_page_offset;
    old_page_offset = old_page.page_offset;
    old_is_leaf = old_page.is_leaf;

    /* If insertion index(left_index) is first
     * as another page in internal page
     */
    if(left_index == -1){
        temp_in_pairs[0].key = key;
        temp_in_pairs[0].page_offset = right_offset;
        for(i = 0; i < old_page.num_keys; i++){
            temp_in_pairs[i + 1].key = old_page.set.in_pair[i].key;
            temp_in_pairs[i + 1].page_offset = old_page.set.in_pair[i].page_offset;
        }
    }
    /* If insertion index is not first */
    else{
        for(i = 0, j = 0; i < old_page.num_keys; i++, j++){
            if(j == left_index) j++;
            temp_in_pairs[j].key = old_page.set.in_pair[i].key;
            temp_in_pairs[j].page_offset = old_page.set.in_pair[i].page_offset;
        }

        temp_in_pairs[left_index].page_offset = right_offset;
        temp_in_pairs[left_index].key = key;
    }

    /* Create the new page and copy
     * half the keys and offset to the
     * old and half to the new
     */
    split = cut(internal_order);
    
    /* Fill old page */
    init_page(&old_page);
    old_page.parent_or_free_page_offset = old_parent_page_offset;
    old_page.page_offset = old_page_offset;
    old_page.is_leaf = old_is_leaf;
    old_page.num_keys = 0;
    
    for(i = 0; i < split - 1; i++){
        old_page.set.in_pair[i].key = temp_in_pairs[i].key;
        old_page.set.in_pair[i].page_offset = temp_in_pairs[i].page_offset;
        old_page.num_keys++;
    }
    
    k_prime = temp_in_pairs[split - 1].key;
  
    /* Fill new page */
    init_page(&new_page); 
    for(++i, j = 0; i < internal_order; i++, j++){
        new_page.set.in_pair[j].page_offset = temp_in_pairs[i].page_offset;
        new_page.set.in_pair[j].key = temp_in_pairs[i].key;
        new_page.num_keys++;
    }
    new_page.page_offset = temp_in_pairs[split - 1].page_offset;

    free(temp_in_pairs);

    new_page.parent_or_free_page_offset = old_page.parent_or_free_page_offset;

    new_offset = is_free_page(table_id);

    if(new_offset == 0){
        head.num_pages++;
        push_header_to_buffer(table_id, get_header_idx(table_id), 0, &head);
        new_offset = (head.num_pages - 1)*PAGESIZE;
        new_idx = make_new_page(table_id);
    }
    else
        new_idx = get_page_from_buffer(table_id, new_offset);

    /* Make new page as parent */

    child_idx = get_page_from_buffer(table_id, new_page.page_offset);
    copy_page(&buf_pool[child_idx].frame_kind.page, &child_page);
    
    child_page.parent_or_free_page_offset = new_offset;
    push_page_to_buffer(table_id, child_idx, new_page.page_offset, &child_page);
   
    for(i = 0; i < new_page.num_keys; i++){
        init_page(&child_page);
        
        child_idx = get_page_from_buffer(table_id, new_page.set.in_pair[i].page_offset);
        copy_page(&buf_pool[child_idx].frame_kind.page, &child_page);

        child_page.parent_or_free_page_offset = new_offset;
        
        push_page_to_buffer(table_id, child_idx, new_page.set.in_pair[i].page_offset, &child_page);
    }
  
    push_page_to_buffer(table_id, old_idx, old_offset, &old_page);
    push_page_to_buffer(table_id, new_idx, new_offset, &new_page);

    insert_into_parent(table_id, old_offset, k_prime, new_offset);

    return;
} 

            
/* Insert a new element into B+ tree*/
void insert_into_parent(int table_id, uint64_t left_offset, int64_t key, 
        uint64_t right_offset){

    int left_index;
    int parent_num_keys;
    int child_idx;
    int parent_idx;
    uint64_t parent_offset;
   
    Page temp_child;
    child_idx = get_page_from_buffer(table_id, left_offset);
    copy_page(&buf_pool[child_idx].frame_kind.page, &temp_child);
    
    parent_offset = temp_child.parent_or_free_page_offset;

    Page temp_parent;
    parent_idx = get_page_from_buffer(table_id, parent_offset);
    copy_page(&buf_pool[parent_idx].frame_kind.page, &temp_parent);
    
    parent_num_keys = temp_parent.num_keys;

    /* Case : new root page */

    if(parent_offset == 0){
       // buf_pool[child_idx].pin_count = 0;
       // buf_pool[parent_idx].pin_count = 0;
        return insert_into_new_root(table_id, left_offset, key, right_offset);
    }
    /* Case : leaf or internal 
     * (Remainder of function body)
     */

    /* Find the parent's offset to left Page */
    left_index = get_left_index(table_id, parent_offset, left_offset);

    /* Simple case : the new key fits into page */
    if(parent_num_keys < internal_order - 1){
        buf_pool[child_idx].pin_count = 0;
        buf_pool[parent_idx].pin_count = 0;

        return insert_into_internal(table_id, parent_offset, left_offset, left_index, key, right_offset);
    }
    /* Harder case: split a page in order
     * to preserve the B+ tree properties
     */
    buf_pool[child_idx].pin_count = 0;
    buf_pool[parent_idx].pin_count = 0;

    return insert_into_internal_after_splitting
        (table_id, parent_offset, left_index, key, right_offset);

}

/* Creates a new root page for two subtress
 * and inserts the appropriate key into
 * the new root page
 */
void insert_into_new_root(int table_id, uint64_t left_offset, int64_t key, uint64_t right_offset){
    uint64_t root_offset;
   
    HeaderPage head; 
    Page root_page;
    Page left_page;
    Page right_page;
    int root_idx;
    int left_idx;
    int right_idx;

    copy_header_page(&buf_pool[get_header_idx(table_id)].frame_kind.head, &head);
    left_idx = get_page_from_buffer(table_id, left_offset);
    copy_page(&buf_pool[left_idx].frame_kind.page, &left_page);
    
    right_idx = get_page_from_buffer(table_id, right_offset);
    copy_page(&buf_pool[right_idx].frame_kind.page, &right_page);

    root_idx = make_new_page(table_id);
    init_page(&root_page);


    root_page.set.in_pair[0].key = key;
    root_page.page_offset = left_offset;
    root_page.set.in_pair[0].page_offset = right_offset;
    root_page.num_keys++;
    root_page.parent_or_free_page_offset = 0;
    root_page.is_leaf = false;


    root_offset = is_free_page(table_id);

    if(root_offset == 0){
        head.num_pages++;
        push_header_to_buffer(table_id, get_header_idx(table_id), 0, &head);
        root_offset = (head.num_pages - 1)*PAGESIZE;
    }

    left_page.parent_or_free_page_offset = root_offset;
    right_page.parent_or_free_page_offset = root_offset;

    copy_header_page(&buf_pool[get_header_idx(table_id)].frame_kind.head, &head);
    head.root_page_offset = root_offset;

    push_header_to_buffer(table_id, get_header_idx(table_id), 0, &head);
    push_page_to_buffer(table_id, left_idx, left_offset, &left_page);
    push_page_to_buffer(table_id, right_idx, right_offset, &right_page);
    push_page_to_buffer(table_id, root_idx, root_offset, &root_page);
    return;
} 

/* First insertion
 * Make new page and save data
 */
void make_first_page(int table_id, int64_t key, char *value){
    HeaderPage head;
    copy_header_page(&buf_pool[get_header_idx(table_id)].frame_kind.head, &head);

    Page temp;
    int idx;
    uint64_t offset;

    idx = make_new_page(table_id);

    temp.parent_or_free_page_offset = 0;
    temp.is_leaf = true;
    temp.num_keys++;
    temp.page_offset = 0;
    temp.set.pair[0].key = key;
    strcpy(temp.set.pair[0].value, value);

    offset = is_free_page(table_id);
    if(offset == 0){
        head.root_page_offset = PAGESIZE;
        head.num_pages++;

        push_header_to_buffer(table_id, get_header_idx(table_id), 0, &head);
        push_page_to_buffer(table_id, idx, PAGESIZE, &temp);
    }  

    else{
        head.root_page_offset = offset;
        
        push_header_to_buffer(table_id, get_header_idx(table_id), 0, &head);
        push_page_to_buffer(table_id, idx, offset, &temp);
    }

    return;
}

/* Insert input ‘key/value’(record) 
 * to data file at the right place
 * Master insertion function
 */
int insert(int table_id, int64_t key, char *value){

    HeaderPage head;
    copy_header_page(&buf_pool[get_header_idx(table_id)].frame_kind.head, &head);

    /* The current implementation
     * ignores dplicates
     */
    if(find(table_id, key) != NULL)
        return -1;

    /* Case : the tree does not exist yet
     * Start new tree
     */
    if(head.root_page_offset == 0){
        make_first_page(table_id, key, value);
        return 0;
    }

    /* Case : the tree already exist
     * (Rest of function body)
     */
    Page temp;
    int idx;
    uint64_t page_offset;

    page_offset = find_leaf_page(table_id, key);
    idx = get_page_from_buffer(table_id, page_offset);
    copy_page(&buf_pool[idx].frame_kind.page, &temp);

    /* Case : leaf has room for key and value */
    if(temp.num_keys < leaf_order -1){
       insert_into_leaf(table_id, key, value, page_offset);
       return 0;
    } 

    /* Case : leaf must be split */
    else{
        insert_into_leaf_after_splitting(table_id, key, value, page_offset);
        return 0;
    }

    return -1;
}


/* Delete */

/* Utility function for deletion
 * Retrives the index of a page nearest
 * neighbor (sibling) to the right
 * if one exists.
 * If not (the page is the rightmost page),
 * returns -1 to signify this special case
 */
int get_neighbor_index(int table_id, uint64_t current_offset){
    int i;
    int cur_idx;
    int par_idx;
    Page current_page;
    Page parent_page;

    cur_idx = get_page_from_buffer(table_id, current_offset);
    copy_page(&buf_pool[cur_idx].frame_kind.page, &current_page);

    par_idx = get_page_from_buffer(table_id, current_page.parent_or_free_page_offset);
    copy_page(&buf_pool[par_idx].frame_kind.page, &parent_page);

    /* Return the index of the key to the right
     * of the offset in the parent offset to 
     * current page
     * If current_offset is the right most page,
     * this means return -1
     */
    if(current_offset == parent_page.page_offset)
        return 0;
    else{
        for(i = 0; i < parent_page.num_keys ; i++){
            if(parent_page.set.in_pair[i].page_offset == current_offset){
                if(i == parent_page.num_keys - 1)
                    return -1;
                else
                    return i + 1;
            }
        }
    }
}

void remove_entry_from_page(int table_id, int64_t key, uint64_t offset){
    int i;

    int idx;
    Page temp;

    idx = get_page_from_buffer(table_id, offset);
    copy_page(&buf_pool[idx].frame_kind.page, &temp);

    // Remove the key and shift other keys accordingly
    i = 0;
    if(temp.is_leaf){
       while(temp.set.pair[i].key != key)
           i++;
       for(++i; i < temp.num_keys; i++){
           temp.set.pair[i - 1].key = temp.set.pair[i].key;
           strcpy(temp.set.pair[i - 1].value, temp.set.pair[i].value);
       }
       memset(&temp.set.pair[i - 1], 0, 128);
    }
    else{
        while(temp.set.in_pair[i].key != key)
            i++;
        for(++i; i < temp.num_keys; i++){
            temp.set.in_pair[i - 1].key = temp.set.in_pair[i].key;
            temp.set.in_pair[i - 1].page_offset = temp.set.in_pair[i].page_offset;
        }
        memset(&temp.set.in_pair[i - 1], 0, 16);
    }

    // One key fewer
    temp.num_keys--;
   
    push_page_to_buffer(table_id, idx, offset, &temp);

    return;
}

void adjust_root(int table_id, uint64_t offset){
    uint64_t new_root_page_offset;
    int old_idx;
    int new_idx;
    HeaderPage head;
    Page root_page; 
    Page new_root_page;

    old_idx = get_page_from_buffer(table_id, offset);
    copy_page(&buf_pool[old_idx].frame_kind.page, &root_page);
    copy_header_page(&buf_pool[get_header_idx(table_id)].frame_kind.head, &head);

    /* Case: nonempty root
     * Key and offset have already been deleted,
     * so nothing to be done
     */
    if(root_page.num_keys > 0){
        push_page_to_buffer(table_id, old_idx, offset, &root_page);
        return;
    }

    /* Case: empty root */

    // If it has a child, promote
    // the first (only) child
    // as the new root

    if(!root_page.is_leaf){
        new_root_page_offset = root_page.page_offset;

        new_idx = get_page_from_buffer(table_id, new_root_page_offset);
        copy_page(&buf_pool[new_idx].frame_kind.page, &new_root_page);

        new_root_page.parent_or_free_page_offset = 0;

        head.root_page_offset = new_root_page_offset;

        init_page(&root_page);

        push_page_to_buffer(table_id, old_idx, offset, &root_page);

        push_header_to_buffer(table_id, get_header_idx(table_id), 0, &head);
        
        push_page_to_buffer(table_id, new_idx, new_root_page_offset, &new_root_page);
        
        insert_into_free_page(table_id, offset);

    }

    // If it is a leaf (has no childern),
    // then the whole tree is empty
    else{
        
        head.root_page_offset = 0;

        init_page(&root_page);

        push_page_to_buffer(table_id, old_idx, offset, &root_page);

        insert_into_free_page(table_id, offset);
        push_header_to_buffer(table_id, get_header_idx(table_id), 0, &head);
    }

    return;
}

void coalesce_pages(int table_id, uint64_t current_offset, uint64_t neighbor_offset, 
        int neighbor_index, int64_t k_prime){
    int i, j, neighbor_insertion_index, p_end;
    uint64_t temp_offset, parent_offset;
  
    int idx; 
    int cur_idx;
    int nei_idx; 
    /* Swap neighbor with page if page is on the
     * extreme right and neighbor is to its left
     */
    if(neighbor_index == -1){
        temp_offset = current_offset;
        current_offset = neighbor_offset;
        neighbor_offset = temp_offset;
    }

    Page current_page;
    cur_idx = get_page_from_buffer(table_id, current_offset);
    copy_page(&buf_pool[cur_idx].frame_kind.page, &current_page); 

    Page neighbor_page;
    nei_idx = get_page_from_buffer(table_id, neighbor_offset);
    copy_page(&buf_pool[nei_idx].frame_kind.page, &neighbor_page);


    /* Starting point in the neighbor for copying
     * keys and offset from current_page
     * Recall that current_page and neighbor_page
     * have swapped places in the spacial
     * case of current_page being a leftmost page
     */
    neighbor_insertion_index = current_page.num_keys;

    /* Case: nonleaf page
     * Append k_prime and the following offset
     * Apped all offsets and keys from the 
     * neighbor_page
     */

    if(!current_page.is_leaf){

        /* Append k_prime */
        current_page.set.in_pair[neighbor_insertion_index].key = k_prime;
        current_page.num_keys++;

        p_end = neighbor_page.num_keys;

        for(i = neighbor_insertion_index + 1, j = 0; j < p_end; i++, j++){
            current_page.set.in_pair[i].key = neighbor_page.set.in_pair[j].key;
            current_page.set.in_pair[i].page_offset = neighbor_page.set.in_pair[j].page_offset;
            current_page.num_keys++;
            neighbor_page.num_keys--;
        }

        /* Current page's right most page offset should be in
         * neighbor_inersirtion_index's page offset
         */
        current_page.set.in_pair[neighbor_insertion_index].page_offset 
            = neighbor_page.page_offset;

        /* All childern must now point up to the same parent */
        Page temp;
        idx = get_page_from_buffer(table_id, current_page.page_offset);
        copy_page(&buf_pool[idx].frame_kind.page, &temp);

        temp.parent_or_free_page_offset = current_offset;
        
        push_page_to_buffer(table_id, idx, current_page.page_offset, &temp);

        init_page(&temp);        
        
        for(i = 0; i < current_page.num_keys; i++){
            idx = get_page_from_buffer(table_id, current_page.set.in_pair[i].page_offset);
            copy_page(&buf_pool[idx].frame_kind.page, &temp);
            
            temp.parent_or_free_page_offset = current_offset;

            push_page_to_buffer(table_id, idx, current_page.set.in_pair[i].page_offset, &temp);
            init_page(&temp);
        }
    }

    /* In a leaf, append the keys and value of
     * neighbor page to current page
     * Set the current page_offset to
     * what had been neighbor's page_offset
     */
    else{
        for(i = neighbor_insertion_index, j = 0; j < neighbor_page.num_keys; i++, j++){
            current_page.set.pair[i].key = neighbor_page.set.pair[j].key;
            strcpy(current_page.set.pair[i].value, neighbor_page.set.pair[j].value);
            current_page.num_keys++;
        }
        current_page.page_offset = neighbor_page.page_offset;
    }

    // Set parent page offset
    parent_offset = current_page.parent_or_free_page_offset;

    init_page(&neighbor_page);

    push_page_to_buffer(table_id, nei_idx, neighbor_offset, &neighbor_page);

    // Insert free page to free list
    insert_into_free_page(table_id, neighbor_offset);

    push_page_to_buffer(table_id, cur_idx, current_offset, &current_page);

    delete_entry(table_id, k_prime, parent_offset);
    return;
}

/* Redistribute entries between two pages when
 * one has become too small after deletion
 * but its neighbor is too big to append the
 * small page's entries without exceeding the
 * maximum
 */
void redistribute_pages(int table_id, uint64_t current_offset, uint64_t neighbor_offset, 
        int neighbor_index, int k_prime_index, uint64_t k_prime){

    int i;
    int cur_idx;
    int nei_idx;
    int par_idx;
    int chi_idx;
    uint64_t temp_offset;

    Page current_page;
    cur_idx = get_page_from_buffer(table_id, current_offset);
    copy_page(&buf_pool[cur_idx].frame_kind.page, &current_page);

    Page neighbor_page;
    nei_idx = get_page_from_buffer(table_id, neighbor_offset);
    copy_page(&buf_pool[nei_idx].frame_kind.page, &neighbor_page);

    Page parent_page;
    par_idx = get_page_from_buffer(table_id, current_page.parent_or_free_page_offset);
    copy_page(&buf_pool[par_idx].frame_kind.page, &parent_page);

    /* Case: current page has a neighbor to the right
     * Pull the neighbor's first key-offset(value) pair over
     * from the neighbor's left end to current page's right end
     */

    if(neighbor_index != -1){
        if(!current_page.is_leaf){
            // Get k prime to current page right end
            current_page.set.in_pair[current_page.num_keys].key
                = k_prime;
            current_page.set.in_pair[current_page.num_keys].page_offset
                = neighbor_page.page_offset;

            parent_page.set.in_pair[k_prime_index].key 
                = neighbor_page.set.in_pair[0].key;
            neighbor_page.page_offset 
                = neighbor_page.set.in_pair[0].page_offset;

            for(i = 0; i < neighbor_page.num_keys - 1; i++){
                neighbor_page.set.in_pair[i].key
                    = neighbor_page.set.in_pair[i + 1].key;
                neighbor_page.set.in_pair[i].page_offset
                    = neighbor_page.set.in_pair[i + 1].page_offset;
            }
            memset(&neighbor_page.set.in_pair[neighbor_page.num_keys - 1], '\0', 16);

            Page child_page;
            chi_idx = get_page_from_buffer(table_id, current_page.set.in_pair[current_page.num_keys].page_offset);
            copy_page(&buf_pool[chi_idx].frame_kind.page, &child_page);

            child_page.parent_or_free_page_offset = current_offset;

            push_page_to_buffer(table_id, chi_idx, current_page.set.in_pair[current_page.num_keys].page_offset, &child_page);
        }

        else{
            current_page.set.pair[current_page.num_keys].key
                = neighbor_page.set.pair[0].key;
            strcpy(current_page.set.pair[current_page.num_keys].value,
                    neighbor_page.set.pair[0].value);

            for(i = 0; i < neighbor_page.num_keys - 1; i++){
                neighbor_page.set.pair[i].key
                    = neighbor_page.set.pair[i + 1].key;
                strcpy(neighbor_page.set.pair[i].value,
                        neighbor_page.set.pair[i + 1].value);
            }

            parent_page.set.in_pair[k_prime_index].key
                = neighbor_page.set.pair[0].key;


            memset(&neighbor_page.set.pair[neighbor_page.num_keys - 1], '\0', 128);
        }

        current_page.num_keys++;
        neighbor_page.num_keys--;

    }

    /* Case: current page is rightmost page
     * Take a key-offset(value) pair from the neighbor to the left
     * Move the neighbor's rightmost key-pointer(value) pair
     * to current page's leftmost position
     */
    else{
        if(!current_page.is_leaf){
            for(i = current_page.num_keys; i > 0; i--){
                current_page.set.in_pair[i].key = current_page.set.in_pair[i - 1].key;
                current_page.set.in_pair[i].page_offset
                    = current_page.set.in_pair[i - 1].page_offset;
            }
            current_page.set.in_pair[0].page_offset = current_page.page_offset;
            current_page.set.in_pair[0].key 
                = parent_page.set.in_pair[k_prime_index].key;
            
            parent_page.set.in_pair[k_prime_index].key
                = neighbor_page.set.in_pair[neighbor_page.num_keys - 1].key;
            current_page.page_offset
                = neighbor_page.set.in_pair[neighbor_page.num_keys - 1].page_offset;

            memset(&neighbor_page.set.in_pair[neighbor_page.num_keys - 1], 0, 16);

            Page child_page;
            chi_idx = get_page_from_buffer(table_id, current_page.page_offset);
            copy_page(&buf_pool[chi_idx].frame_kind.page, &child_page);

            child_page.parent_or_free_page_offset = current_offset;

            push_page_to_buffer(table_id, chi_idx, current_page.page_offset, &child_page);
        }

        else{
            for(i = current_page.num_keys; i > 0; i--){
                current_page.set.pair[i].key = current_page.set.pair[i - 1].key;
                strcpy(current_page.set.pair[i].value, current_page.set.pair[i - 1].value);
            }

            current_page.set.pair[0].key 
                = neighbor_page.set.pair[neighbor_page.num_keys - 1].key;
            strcpy(current_page.set.pair[0].value,
                    neighbor_page.set.pair[neighbor_page.num_keys - 1].value);

            parent_page.set.in_pair[k_prime_index].key
                = current_page.set.pair[0].key;

            memset(&neighbor_page.set.pair[neighbor_page.num_keys - 1], 0, 128);
        }

        current_page.num_keys++;
        neighbor_page.num_keys--;
    }

    push_page_to_buffer(table_id, par_idx, current_page.parent_or_free_page_offset, &parent_page);
    push_page_to_buffer(table_id, cur_idx, current_offset, &current_page);
    push_page_to_buffer(table_id, nei_idx, neighbor_offset, &neighbor_page);

    return;
}

/* Deletes an entry from the page
 * Removes the key and value from leaf page,
 * and then makes all appropriate
 * changes to preserve the B+ tree properties
 */
void delete_entry(int table_id, int64_t key, uint64_t offset){
    int min_keys;
    int neighbor_index;
    int k_prime_index;
    int64_t k_prime;
    int capacity;
    uint64_t neighbor_offset;

    int nei_idx;
    int par_idx;
    int cur_idx;
    HeaderPage head;
    Page current_page;

    copy_header_page(&buf_pool[get_header_idx(table_id)].frame_kind.head, &head);

    remove_entry_from_page(table_id, key, offset);

    /* Case : deletion from the root */
    if(offset == head.root_page_offset){
        return adjust_root(table_id, offset);
    }
    
    /* Case : deletion from a page below the root page
     * (Rest of the function body)
     */

    /* Determine minimum allowable size of page,
     * to be preserved after deletion
     */
    cur_idx = get_page_from_buffer(table_id, offset);
    copy_page(&buf_pool[cur_idx].frame_kind.page, &current_page);

    min_keys = current_page.is_leaf ? cut(leaf_order) -1 : cut(internal_order) - 1;

    /* Case: page stays at or above minimum
     * (The simple case)
     */
    if(current_page.num_keys >= min_keys){
        buf_pool[cur_idx].pin_count = 0;
        return;
    }

    /* Case : page falls below minimum
     * Either coalescence or redistribution
     * is needed
     */

    /* Find the appropriate neighbor page with which
     * to coalesce
     * Also find the key (k_prime) in the parent
     * between the pointer to page temp and the offset
     * to the neighbor
     */

    Page parent_page;
    
    par_idx = get_page_from_buffer(table_id, current_page.parent_or_free_page_offset);
    copy_page(&buf_pool[par_idx].frame_kind.page, &parent_page);

    neighbor_index = get_neighbor_index(table_id, offset);

    switch(neighbor_index){
        case -1:
            k_prime_index = parent_page.num_keys - 1;
            if(k_prime_index == 0)
                neighbor_offset = parent_page.page_offset;
            else
                neighbor_offset = parent_page.set.in_pair[k_prime_index - 1].page_offset;
            break;
        
        default:
            k_prime_index = neighbor_index;
            neighbor_offset = parent_page.set.in_pair[k_prime_index].page_offset;
            break;
    }
    k_prime = parent_page.set.in_pair[k_prime_index].key;

    capacity = current_page.is_leaf ? leaf_order -1  : internal_order - 1;

    Page neighbor_page;
    nei_idx = get_page_from_buffer(table_id, neighbor_offset);
    copy_page(&buf_pool[nei_idx].frame_kind.page, &neighbor_page);

    /* Coalescence */
    if((neighbor_page.num_keys + current_page.num_keys) < capacity){
        coalesce_pages(table_id, offset, neighbor_offset, neighbor_index, k_prime);
        return;
    }

    /* Redistribution */
    else{
        redistribute_pages(table_id, offset, neighbor_offset, neighbor_index, k_prime_index, k_prime);
        return;
    }
}


/* Master deletion function */
int delete(int table_id, int64_t key){
    uint64_t offset;

    if(find(table_id, key) == NULL){
        return -1;
    }
    
    offset = find_leaf_page(table_id, key);

    delete_entry(table_id, key, offset);
    return 0;
}
