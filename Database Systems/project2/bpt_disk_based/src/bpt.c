#include "bpt.h"
#include "pages.h"

int leaf_order = LEAF_ORDER;
int internal_order = INTERNAL_ORDER;

// Helper Function

/* Print out current file */
void print_file(){
    printf("######################################################\n");
    int size;
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
void print_tree() {
    Page page_;
    Page *page;
    page = &page_;
    //page = (Page*)calloc(1, PAGESIZE);
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

/* Read page from disk and
 * load page to temp page
 */
void load_page(FILE * fp, int64_t page_offset, Page * temp){
    fseek(fp, page_offset, SEEK_SET);
    fread(temp, 1, PAGESIZE, fp);
}

/* Initialize page */
void init_head_page(){
    memset(headPage, 0, PAGESIZE);
}
void init_page(Page *temp){
    memset(temp, 0, PAGESIZE);
}

// Free list

/* Insert free pgae into free list */
void insert_into_free_page(uint64_t offset){

    Page new_;
    Page *new;
    new = &new_;
    load_page(fp, offset, new);

    /* Case: first free page */
    if(headPage->free_page_offset == 0){
        headPage->free_page_offset = offset;
        new->parent_or_free_page_offset = 0;
    }

    /* Case: insert new free page 
     * into free page list
     * (Rest of the function body)
     */
    else{
        new->parent_or_free_page_offset = headPage->free_page_offset;
        headPage->free_page_offset = offset;
    }

    rewind(fp);
    fwrite(headPage, 1, PAGESIZE, fp);
    fseek(fp, offset, SEEK_SET);
    fwrite(new, 1, PAGESIZE, fp);
    fflush(fp);

    return;
}

/* Look ahead header page's free page offset
 * If free page existing, use for page 
 * and return free page's offset
 * else return
 */
uint64_t is_free_page(void){

    /* Case: empty free page list */
    if(headPage->free_page_offset == 0)
        return 0;

    /* Case: free page exists
     * (Rest of the function body)
     */
    else{
        uint64_t offset;
        Page temp_;
        Page * temp;
        temp = &temp_;
        
        offset = headPage->free_page_offset;
        load_page(fp, headPage->free_page_offset, temp);

        headPage->free_page_offset = temp->parent_or_free_page_offset;

        rewind(fp);
        fwrite(headPage, 1, PAGESIZE, fp);
        fseek(fp, offset, SEEK_SET);
        fwrite(temp, 1, PAGESIZE, fp);
        fflush(fp);

        return offset;
    }
}


// Open

/* Open existing data file using
 * ‘pathname’ or create one
 * if not existed
 */
int open_db(char *pathname){
    fp = NULL;

    fp = fopen(pathname, "r+b");

    /* Create data file and
     * initialize header file
     * and return 0 */
    if(fp == NULL){
        fp = fopen(pathname, "w+b");
        headPage = (HeaderPage*)calloc(1, PAGESIZE);
        headPage->num_pages = 1;
        fwrite(headPage, 1, PAGESIZE, fp);
        fflush(fp);
        return 0;
    }

    /* Open existing data file and
     * read header page from file
     * and return 0 */
    else{
        headPage = (HeaderPage*)calloc(1, PAGESIZE);
        rewind(fp);
        fread(headPage, 1, PAGESIZE, fp);
        return 0;
    }

    /* If fail, return -1 */
    return -1;
}

// FIND

/* Find leaf page */
uint64_t find_leaf_page(int64_t key){

    int i = 0;
    uint64_t offset;

    // Initailize file pointer to top of file
    rewind(fp);
    Page temp_;
    Page *temp;
    temp = &temp_;

    // Get root page offset
    int64_t root_page_offset = headPage->root_page_offset;

    // Load root page to temp page
    load_page(fp, root_page_offset, temp);
    
    offset = root_page_offset;

    if(headPage->root_page_offset == 0)
       return offset;

    // Find until meet leaf page
    while(!temp->is_leaf){
        i = 0;
        if(key < temp->set.in_pair[0].key){
            offset = temp->page_offset;
            init_page(temp);
            load_page(fp, offset, temp);
        }
        else{
            while(i < temp->num_keys){
                if(key >= temp->set.in_pair[i].key) i++;
                else break;
            }
            offset = temp->set.in_pair[i-1].page_offset;
            init_page(temp);
            load_page(fp, offset, temp);
        } 
    }
    return offset;
}

/* Find the record containing input 'key' */
char * find(int64_t key){
    
    // Allocate temp page
    Page temp_;
    Page * temp;
    temp = &temp_;
    //temp = (Page*)calloc(1, PAGESIZE);
    uint64_t offset;

    // Find leaf page
    offset = find_leaf_page(key);
    load_page(fp, offset, temp);
    
    // Check for existing key
    int i = 0;
    for(i = 0; i < temp->num_keys; i++)
        if(temp->set.pair[i].key == key)
            break;

    // Fail for finding key
    if(i == temp->num_keys)
        return NULL;

    // Success for finding key
    else{
        memset(BUF, 0, 120);
        strcpy(BUF, temp->set.pair[i].value);
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

// Insert


/* Creates a new general page, which can be adapted
 * to serve as either a leaf or an internal page
 */
Page * make_new_page(void){
    Page * newPage;
    newPage = (Page*)calloc(1,PAGESIZE);
    return newPage;
}

/* Create a new leaf page by creating a new page
 * and then adapting it appropriately
 */
Page * make_leaf_page(void){
    Page * leaf = make_new_page();
    leaf->is_leaf = true;
    return leaf;
}

/* Helper function used in insert_into_parent
 * ro find the index of the parent's offset to
 * the page to the left of the key to be inserted
 */
int get_left_index(uint64_t parent_offset, uint64_t left_offset){

    Page parent_;
    Page * parent;
    parent = &parent_;
    load_page(fp, parent_offset, parent);

    Page left_;
    Page * left;
    left = &left_;
    load_page(fp, left_offset, left);

    int left_index = 0;

    if(left_offset == parent->page_offset)
        return -1;
    else{
        while(left_index <= parent->num_keys &&
            parent->set.in_pair[left_index].page_offset != left_offset)
            left_index++;
        
        left_index++;
        
        return left_index;
    }
}

/* Inserts a new key and value to leaf page*/
void insert_into_leaf(int64_t key, char *value, uint64_t page_offset){
    int i, insertion_point;

    Page temp_;
    Page *temp;
    temp = &temp_;
    load_page(fp, page_offset, temp);

    insertion_point = 0;

    while(insertion_point < temp->num_keys && 
            temp->set.pair[insertion_point].key < key)
        insertion_point++;

    for(i = temp->num_keys; i > insertion_point; i--){
        temp->set.pair[i].key = temp->set.pair[i-1].key;
        strcpy(temp->set.pair[i].value, temp->set.pair[i-1].value);
    }
    temp->set.pair[insertion_point].key = key;
    strcpy(temp->set.pair[insertion_point].value, value);
    temp->num_keys++;

    fseek(fp, page_offset, SEEK_SET);
    fwrite(temp, 1, PAGESIZE, fp);
    fflush(fp);
    
    return;
}

/* Insert a new key and value
 * to a new leaf page so as to exceed
 * the tree's order, causing the leaf to be split
 * in half
 */
void insert_into_leaf_after_splitting(int64_t key, char *value, uint64_t old_offset){
    Page old_leaf_;
    Page *old_leaf;
    Page *new_leaf;
    Pair *temp_pairs;
    old_leaf = &old_leaf_;
    int insertion_index, split, i, j;
    uint64_t old_parent_page_offset;
    int old_is_leaf;
    uint64_t old_page_offset;
    uint64_t new_offset;
    int64_t new_key;

    load_page(fp, old_offset, old_leaf);
    
    old_parent_page_offset = old_leaf->parent_or_free_page_offset;
    old_is_leaf = old_leaf->is_leaf;
    old_page_offset = old_leaf->page_offset;

    new_leaf = make_leaf_page();

    temp_pairs = (Pair*)calloc(leaf_order, sizeof(Pair));

    insertion_index = 0;

    while(insertion_index < leaf_order - 1 && old_leaf->set.pair[insertion_index].key < key)
        insertion_index++;

    for(i = 0, j = 0; i < old_leaf->num_keys; i++, j++){
        if(j == insertion_index) j++;
        temp_pairs[j].key = old_leaf->set.pair[i].key;
        strcpy(temp_pairs[j].value, old_leaf->set.pair[i].value);
    }

    temp_pairs[insertion_index].key = key;
    strcpy(temp_pairs[insertion_index].value, value);

    init_page(old_leaf);
    old_leaf->parent_or_free_page_offset = old_parent_page_offset;
    old_leaf->is_leaf = old_is_leaf; 
    old_leaf->page_offset = old_page_offset;

    old_leaf->num_keys = 0;

    split = cut(leaf_order - 1);

    for(i = 0; i < split; i++){
        old_leaf->set.pair[i].key = temp_pairs[i].key;
        strcpy(old_leaf->set.pair[i].value, temp_pairs[i].value);
        old_leaf->num_keys++;
    }

    for(i = split, j= 0; i < leaf_order; i++, j++){
        new_leaf->set.pair[j].key = temp_pairs[i].key;
        strcpy(new_leaf->set.pair[j].value, temp_pairs[i].value);
        new_leaf->num_keys++;
    }

    free(temp_pairs);

    new_leaf->page_offset = old_leaf->page_offset;
    new_leaf->is_leaf = true;

    /* Check if there is free page */

    new_offset = is_free_page();

    // If there is a free page?
    if(new_offset != 0)
        old_leaf->page_offset = new_offset;

    // Else no free page
    else{
        headPage->num_pages++;
        rewind(fp);
        fwrite(headPage, 1, PAGESIZE, fp);
        old_leaf->page_offset = (headPage->num_pages - 1)*PAGESIZE;
    }

    new_leaf->parent_or_free_page_offset = old_leaf->parent_or_free_page_offset;
    new_key = new_leaf->set.pair[0].key;

    fseek(fp, old_offset, SEEK_SET);
    fwrite(old_leaf, 1, PAGESIZE, fp);

    fseek(fp, old_leaf->page_offset, SEEK_SET);
    fwrite(new_leaf, 1, PAGESIZE, fp);
    fflush(fp);

    new_offset = old_leaf->page_offset;

    free(new_leaf);

    insert_into_parent(old_offset, new_key, new_offset);

    return;
}

/* Inserts a new key and offset to a page
 * into a page into which these can fit
 * without violating the B+ tree properties
 */
void insert_into_internal(uint64_t parent_offset, uint64_t left_offset,
        int left_index, int64_t key, uint64_t right_offset) {
   
    Page temp_; 
    Page *temp;
    temp = &temp_;
    load_page(fp, parent_offset, temp);

    int i;
    if(left_index == -1){
        for(i = temp->num_keys; i > 0; i--){
            temp->set.in_pair[i].page_offset = temp->set.in_pair[i-1].page_offset;
            temp->set.in_pair[i].key = temp->set.in_pair[i-1].key;
        }

        temp->page_offset = left_offset;
        temp->set.in_pair[0].key = key;
        temp->set.in_pair[0].page_offset = right_offset;
    } 

    else{
        for(i = temp->num_keys; i > left_index; i--){
            temp->set.in_pair[i].page_offset = temp->set.in_pair[i-1].page_offset;
            temp->set.in_pair[i].key = temp->set.in_pair[i-1].key;
        }
        temp->set.in_pair[left_index].page_offset = right_offset;
        temp->set.in_pair[left_index].key = key;
    }

    temp->is_leaf = false;
    temp->num_keys++;

    fseek(fp, parent_offset, SEEK_SET);
    fwrite(temp, 1, PAGESIZE, fp);
    fflush(fp);

    return;
}

/* Inserts a new key and offset to a page
 * into a page, causing the pages's size to exceed
 * the order, and causing the page to split into two
 */
void insert_into_internal_after_splitting(uint64_t old_offset, 
        int left_index, int64_t key, uint64_t right_offset){

    int i, j, split; 
    int64_t k_prime;
    uint64_t new_offset, old_parent_page_offset, old_page_offset;
    int old_is_leaf;
    Page * new_page;
    Page child_page_;
    Page * child_page;
    child_page = &child_page_;
    Page old_page_;
    Page * old_page;
    old_page = &old_page_;
    InPair * temp_in_pairs;

    /* First create a temporary set of keys and offsets
     * to hold everything in order, including
     * the new key and offset, inserted in their
     * correct places
     * Then create a new page and copy half of the
     * keys and offsets to the old page and
     * the other half to the new
     */

    temp_in_pairs = (InPair*)calloc(internal_order, sizeof(InPair));
    load_page(fp, old_offset, old_page);
    
    old_parent_page_offset = old_page->parent_or_free_page_offset;
    old_page_offset = old_page->page_offset;
    old_is_leaf = old_page->is_leaf;

    /* If insertion index(left_index) is first
     * as another page in internal page
     */
    if(left_index == -1){
        temp_in_pairs[0].key = key;
        temp_in_pairs[0].page_offset = right_offset;
        for(i = 0; i < old_page->num_keys; i++){
            temp_in_pairs[i + 1].key = old_page->set.in_pair[i].key;
            temp_in_pairs[i + 1].page_offset = old_page->set.in_pair[i].page_offset;
        }
    }
    /* If insertion index is not first */
    else{
        for(i = 0, j = 0; i < old_page->num_keys; i++, j++){
            if(j == left_index) j++;
            temp_in_pairs[j].key = old_page->set.in_pair[i].key;
            temp_in_pairs[j].page_offset = old_page->set.in_pair[i].page_offset;
        }

        temp_in_pairs[left_index].page_offset = right_offset;
        temp_in_pairs[left_index].key = key;
    }

    /* Create the new page and copy
     * half the keys and offset to the
     * old and half to the new
     */
    split = cut(internal_order);
    new_page = make_new_page();
    
    /* Fill old page */
    init_page(old_page);
    old_page->parent_or_free_page_offset = old_parent_page_offset;
    old_page->page_offset = old_page_offset;
    old_page->is_leaf = old_is_leaf;
    old_page->num_keys = 0;
    
    for(i = 0; i < split - 1; i++){
        old_page->set.in_pair[i].key = temp_in_pairs[i].key;
        old_page->set.in_pair[i].page_offset = temp_in_pairs[i].page_offset;
        old_page->num_keys++;
    }
    
    k_prime = temp_in_pairs[split - 1].key;
  
    /* Fill new page */
    init_page(new_page); 
    for(++i, j = 0; i < internal_order; i++, j++){
        new_page->set.in_pair[j].page_offset = temp_in_pairs[i].page_offset;
        new_page->set.in_pair[j].key = temp_in_pairs[i].key;
        new_page->num_keys++;
    }
    new_page->page_offset = temp_in_pairs[split - 1].page_offset;

    free(temp_in_pairs);

    new_page->parent_or_free_page_offset = old_page->parent_or_free_page_offset;

    new_offset = is_free_page();

    if(new_offset == 0){
        headPage->num_pages++;
        rewind(fp);
        fwrite(headPage, 1, PAGESIZE, fp);
        new_offset = (headPage->num_pages - 1)*PAGESIZE;
    }


    /* Make new page as parent */

    load_page(fp, new_page->page_offset, child_page);
    child_page->parent_or_free_page_offset = new_offset;
    fseek(fp, new_page->page_offset, SEEK_SET);
    fwrite(child_page, 1, PAGESIZE, fp);

   
    for(i = 0; i < new_page->num_keys; i++){
        init_page(child_page);
        load_page(fp, new_page->set.in_pair[i].page_offset, child_page);
        child_page->parent_or_free_page_offset = new_offset;
        fseek(fp, new_page->set.in_pair[i].page_offset, SEEK_SET);
        fwrite(child_page, 1, PAGESIZE, fp);
        fflush(fp);
    }
   
    fseek(fp, old_offset, SEEK_SET);
    fwrite(old_page, 1, PAGESIZE, fp);
    fseek(fp, new_offset, SEEK_SET);
    fwrite(new_page, 1, PAGESIZE, fp);
    fflush(fp);
    
    free(new_page);
    

    insert_into_parent(old_offset, k_prime, new_offset);

    return;
} 

            
/* Insert a new element into B+ tree*/
void insert_into_parent(uint64_t left_offset, int64_t key, 
        uint64_t right_offset){

    int left_index;
    int parent_num_keys;
    uint64_t parent_offset;
   
    Page temp_child_; 
    Page * temp_child;
    temp_child = &temp_child_;
    load_page(fp, left_offset, temp_child);
    parent_offset = temp_child->parent_or_free_page_offset;

    Page temp_parent_;
    Page *temp_parent;
    temp_parent = &temp_parent_;
    load_page(fp, parent_offset, temp_parent);
    parent_num_keys = temp_parent->num_keys;

    /* Case : new root page */

    if(parent_offset == 0)
        return insert_into_new_root(left_offset, key, right_offset);

    /* Case : leaf or internal 
     * (Remainder of function body)
     */

    /* Find the parent's offset to left Page */
    left_index = get_left_index(parent_offset, left_offset);

    /* Simple case : the new key fits into page */
    if(parent_num_keys < internal_order - 1)
        return insert_into_internal(parent_offset, left_offset, left_index, key, right_offset);

    /* Harder case: split a page in order
     * to preserve the B+ tree properties
     */
    return insert_into_internal_after_splitting
            (parent_offset, left_index, key, right_offset);
  
}

/* Creates a new root page foe two subtress
 * and inserts the appropriate key into
 * the new root page
 */
void insert_into_new_root(uint64_t left_offset, int64_t key, uint64_t right_offset){
    uint64_t root_offset;
    Page * root_page = make_new_page();
    Page left_page_;
    Page * left_page;
    left_page = &left_page_;

    Page right_page_;
    Page * right_page;
    right_page = &right_page_;

    load_page(fp, left_offset, left_page);
    load_page(fp, right_offset, right_page);

    root_page->set.in_pair[0].key = key;
    root_page->page_offset = left_offset;
    root_page->set.in_pair[0].page_offset = right_offset;
    root_page->num_keys++;
    root_page->parent_or_free_page_offset = 0;
    root_page->is_leaf = false;


    root_offset = is_free_page();

    if(root_offset == 0){
        headPage->num_pages++;
        rewind(fp);
        fwrite(headPage, 1, PAGESIZE, fp);
        root_offset = (headPage->num_pages - 1)*PAGESIZE;
    }

    left_page->parent_or_free_page_offset = root_offset;
    right_page->parent_or_free_page_offset = root_offset;
    headPage->root_page_offset = root_offset;

    rewind(fp);
    fwrite(headPage, 1, PAGESIZE, fp);
    fseek(fp, left_offset, SEEK_SET);
    fwrite(left_page, 1, PAGESIZE, fp);
    fseek(fp, right_offset, SEEK_SET);
    fwrite(right_page, 1, PAGESIZE, fp);
    fseek(fp, root_offset, SEEK_SET);
    fwrite(root_page, 1, PAGESIZE, fp);
    fflush(fp);

    free(root_page);

    return;
} 

/* First insertion
 * Make new page and save data
 */
void make_first_page(int64_t key, char *value){
    Page temp_;
    Page *temp;
    temp = &temp_;
    uint64_t offset;
    
    temp->parent_or_free_page_offset = 0;
    temp->is_leaf = true;
    temp->num_keys++;
    temp->set.pair[0].key = key;
    strcpy(temp->set.pair[0].value, value);

    offset = is_free_page();

    if(offset == 0){
        headPage->root_page_offset = PAGESIZE;
        headPage->num_pages++;
        rewind(fp);
        fwrite(headPage, 1, PAGESIZE, fp);
        fwrite(temp, 1, PAGESIZE, fp);
    }  

    else{
        headPage->root_page_offset = offset;
        rewind(fp);
        fwrite(headPage, 1, PAGESIZE, fp);
    
        fseek(fp, offset, SEEK_SET);
        fwrite(temp, 1, PAGESIZE, fp);
    }

    fflush(fp);

    return;
}

/* Insert input ‘key/value’(record) 
 * to data file at the right place
 * Master insertion function
 */
int insert(int64_t key, char *value){

    /* The current implementation
     * ignores dplicates
     */
    if(find(key) != NULL)
        return -1;

    /* Case : the tree does not exist yet
     * Start new tree
     */
    if(headPage->root_page_offset == 0){
        make_first_page(key, value);
        return 0;
    }

    /* Case : the tree already exist
     * (Rest of function body)
     */
    Page temp_;
    Page *temp;
    temp = &temp_;

    uint64_t page_offset;
    page_offset = find_leaf_page(key);
    load_page(fp, page_offset, temp);

    /* Case : leaf has room for key and value */
    if(temp->num_keys < leaf_order -1){
       insert_into_leaf(key, value, page_offset);
       return 0;
    } 

    /* Case : leaf must be split */
    else{
        insert_into_leaf_after_splitting(key, value, page_offset);
        return 0;
    }

    return -1;
}


// Delete

/* Utility function for deletion
 * Retrives the index of a page nearest
 * neighbor (sibling) to the right
 * if one exists.
 * If not (the page is the rightmost page),
 * returns -1 to signify this special case
 */
int get_neighbor_index(uint64_t current_offset){
    int i;

    Page current_page_;
    Page *current_page;
    current_page = &current_page_;
    load_page(fp, current_offset, current_page);

    Page parent_page_;
    Page *parent_page;
    parent_page = &parent_page_;
    load_page(fp, current_page->parent_or_free_page_offset, parent_page);

    /* Return the index of the key to the right
     * of the offset in the parent offset to 
     * current page
     * If current_offset is the right most page,
     * this means return -1
     */
    if(current_offset == parent_page->page_offset)
        return 0;
    else{
        for(i = 0; i < parent_page->num_keys ; i++){
            if(parent_page->set.in_pair[i].page_offset == current_offset){
                if(i == parent_page->num_keys - 1)
                    return -1;
                else
                    return i + 1;
            }
        }
    }
}

void remove_entry_from_page(int64_t key, uint64_t offset){
    int i;

    Page temp_;
    Page *temp;
    temp = &temp_;
    load_page(fp, offset, temp);

    // Remove the key and shift other keys accordingly
    i = 0;
    if(temp->is_leaf){
       while(temp->set.pair[i].key != key)
           i++;
       for(++i; i < temp->num_keys; i++){
           temp->set.pair[i - 1].key = temp->set.pair[i].key;
           strcpy(temp->set.pair[i - 1].value, temp->set.pair[i].value);
       }
       memset(&temp->set.pair[i - 1], 0, 128);
    }
    else{
        while(temp->set.in_pair[i].key != key)
            i++;
        for(++i; i < temp->num_keys; i++){
            temp->set.in_pair[i - 1].key = temp->set.in_pair[i].key;
            temp->set.in_pair[i - 1].page_offset = temp->set.in_pair[i].page_offset;
        }
        memset(&temp->set.in_pair[i - 1], 0, 16);
    }

    // One key fewer
    temp->num_keys--;
    
    fseek(fp, offset, SEEK_SET);
    fwrite(temp, 1, PAGESIZE, fp);
    fflush(fp);

    return;
}

void adjust_root(uint64_t offset){
    uint64_t new_root_page_offset;
    Page root_page_, new_root_page_;
    Page *root_page, *new_root_page;
    root_page = &root_page_;
    new_root_page = &new_root_page_;
    load_page(fp, offset, root_page);

    /* Case: nonempty root
     * Key and offset have already been deleted,
     * so nothing to be done
     */
    if(root_page->num_keys > 0)
        return;

    /* Case: empty root */

    // If it has a child, promote
    // the first (only) child
    // as the new root

    if(!root_page->is_leaf){
        new_root_page_offset = root_page->page_offset;

        load_page(fp, new_root_page_offset, new_root_page);

        new_root_page->parent_or_free_page_offset = 0;

        headPage->root_page_offset = new_root_page_offset;

        init_page(root_page);

        fseek(fp, offset, SEEK_SET);
        fwrite(root_page, 1, PAGESIZE, fp);

        insert_into_free_page(offset);

        rewind(fp);
        fwrite(headPage, 1, PAGESIZE, fp);

        fseek(fp, new_root_page_offset, SEEK_SET);
        fwrite(new_root_page, 1, PAGESIZE, fp);
        fflush(fp);
    }

    // If it is a leaf (has no childern),
    // then the whole tree is empty
    else{
        
        headPage->root_page_offset = 0;

        init_page(root_page);

        fseek(fp, offset, SEEK_SET);
        fwrite(root_page, 1, PAGESIZE, fp);

        insert_into_free_page(offset);

        rewind(fp);
        fwrite(headPage, 1, PAGESIZE, fp);

        fflush(fp);
    }

    return;
}

void coalesce_pages(uint64_t current_offset, uint64_t neighbor_offset, 
        int neighbor_index, int64_t k_prime){
    int i, j, neighbor_insertion_index, p_end;
    uint64_t temp_offset, parent_offset;
    
    /* Swap neighbor with page if page is on the
     * extreme right and neighbor is to its left
     */
    if(neighbor_index == -1){
        temp_offset = current_offset;
        current_offset = neighbor_offset;
        neighbor_offset = temp_offset;
    }

    Page current_page_;
    Page *current_page;
    current_page = &current_page_;
    load_page(fp, current_offset, current_page);

    Page neighbor_page_;
    Page *neighbor_page;
    neighbor_page = &neighbor_page_;
    load_page(fp, neighbor_offset, neighbor_page);


    /* Starting point in the neighbor for copying
     * keys and offset from current_page
     * Recall that current_page and neighbor_page
     * have swapped places in the spacial
     * case of current_page being a leftmost page
     */
    neighbor_insertion_index = current_page->num_keys;

    /* Case: nonleaf page
     * Append k_prime and the following offset
     * Apped all offsets and keys from the 
     * neighbor_page
     */

    if(!current_page->is_leaf){

        /* Append k_prime */
        current_page->set.in_pair[neighbor_insertion_index].key = k_prime;
        current_page->num_keys++;

        p_end = neighbor_page->num_keys;

        for(i = neighbor_insertion_index + 1, j = 0; j < p_end; i++, j++){
            current_page->set.in_pair[i].key = neighbor_page->set.in_pair[j].key;
            current_page->set.in_pair[i].page_offset = neighbor_page->set.in_pair[j].page_offset;
            current_page->num_keys++;
            neighbor_page->num_keys--;
        }

        /* Current page's right most page offset should be in
         * neighbor_inersirtion_index's page offset
         */
        current_page->set.in_pair[neighbor_insertion_index].page_offset 
            = neighbor_page->page_offset;

        /* All childern must now point up to the same parent */
        Page temp_;
        Page *temp;
        temp = &temp_;
        load_page(fp, current_page->page_offset, temp);
        temp->parent_or_free_page_offset = current_offset;
        fseek(fp, current_page->page_offset, SEEK_SET);
        fwrite(temp, 1, PAGESIZE, fp);
        init_page(temp);        
        for(i = 0; i < current_page->num_keys; i++){
            load_page(fp, current_page->set.in_pair[i].page_offset, temp);
            temp->parent_or_free_page_offset = current_offset;
            fseek(fp, current_page->set.in_pair[i].page_offset, SEEK_SET);
            fwrite(temp, 1, PAGESIZE, fp);
            init_page(temp);
        }
    }

    /* In a leaf, append the keys and value of
     * neighbor page to current page
     * Set the current page_offset to
     * what had been neighbor's page_offset
     */
    else{
        for(i = neighbor_insertion_index, j = 0; j < neighbor_page->num_keys; i++, j++){
            current_page->set.pair[i].key = neighbor_page->set.pair[j].key;
            strcpy(current_page->set.pair[i].value, neighbor_page->set.pair[j].value);
            current_page->num_keys++;
        }
        current_page->page_offset = neighbor_page->page_offset;
    }

    // Set parent page offset
    parent_offset = current_page->parent_or_free_page_offset;

    init_page(neighbor_page);

    fseek(fp, neighbor_offset, SEEK_SET);
    fwrite(neighbor_page, 1, PAGESIZE, fp);

    // Insert free page to free list
    insert_into_free_page(neighbor_offset);

    fseek(fp, current_offset, SEEK_SET);
    fwrite(current_page, 1, PAGESIZE, fp);
    fflush(fp);

    delete_entry(k_prime, parent_offset);
    return;
}

/* Redistribute entries between two pages when
 * one has become too small after deletion
 * but its neighbor is too big to append the
 * small page's entries without exceeding the
 * maximum
 */
void redistribute_pages(uint64_t current_offset, uint64_t neighbor_offset, 
        int neighbor_index, int k_prime_index, uint64_t k_prime){

    int i;
    uint64_t temp_offset;

    Page current_page_;
    Page *current_page;
    current_page =  &current_page_;
    load_page(fp, current_offset, current_page);

    Page neighbor_page_;
    Page *neighbor_page;
    neighbor_page = &neighbor_page_;
    load_page(fp, neighbor_offset, neighbor_page);

    Page parent_page_;
    Page *parent_page;
    parent_page = &parent_page_;
    load_page(fp, current_page->parent_or_free_page_offset, parent_page);

    /* Case: current page has a neighbor to the right
     * Pull the neighbor's first key-offset(value) pair over
     * from the neighbor's left end to current page's right end
     */

    if(neighbor_index != -1){
        if(!current_page->is_leaf){
            // Get k prime to current page right end
            current_page->set.in_pair[current_page->num_keys].key
                = k_prime;
            current_page->set.in_pair[current_page->num_keys].page_offset
                = neighbor_page->page_offset;

            parent_page->set.in_pair[k_prime_index].key 
                = neighbor_page->set.in_pair[0].key;
            neighbor_page->page_offset 
                = neighbor_page->set.in_pair[0].page_offset;

            for(i = 0; i < neighbor_page->num_keys - 1; i++){
                neighbor_page->set.in_pair[i].key
                    = neighbor_page->set.in_pair[i + 1].key;
                neighbor_page->set.in_pair[i].page_offset
                    = neighbor_page->set.in_pair[i + 1].page_offset;
            }
            memset(&neighbor_page->set.in_pair[neighbor_page->num_keys - 1], '\0', 16);

            Page child_page_;
            Page *child_page;
            child_page = &child_page_;
            load_page(fp, current_page->set.in_pair[current_page->num_keys].page_offset, child_page);

            child_page->parent_or_free_page_offset = current_offset;
            fseek(fp, current_page->set.in_pair[current_page->num_keys].page_offset, SEEK_SET);
            fwrite(child_page, 1, PAGESIZE, fp);
        }

        else{
            current_page->set.pair[current_page->num_keys].key
                = neighbor_page->set.pair[0].key;
            strcpy(current_page->set.pair[current_page->num_keys].value,
                    neighbor_page->set.pair[0].value);

            for(i = 0; i < neighbor_page->num_keys - 1; i++){
                neighbor_page->set.pair[i].key
                    = neighbor_page->set.pair[i + 1].key;
                strcpy(neighbor_page->set.pair[i].value,
                        neighbor_page->set.pair[i + 1].value);
            }

            parent_page->set.in_pair[k_prime_index].key
                = neighbor_page->set.pair[0].key;


            memset(&neighbor_page->set.pair[neighbor_page->num_keys - 1], '\0', 128);
        }

        current_page->num_keys++;
        neighbor_page->num_keys--;

    }

    /* Case: current page is rightmost page
     * Take a key-offset(value) pair from the neighbor to the left
     * Move the neighbor's rightmost key-pointer(value) pair
     * to current page's leftmost position
     */
    else{
        if(!current_page->is_leaf){
            for(i = current_page->num_keys; i > 0; i--){
                current_page->set.in_pair[i].key = current_page->set.in_pair[i - 1].key;
                current_page->set.in_pair[i].page_offset
                    = current_page->set.in_pair[i - 1].page_offset;
            }
            current_page->set.in_pair[0].page_offset = current_page->page_offset;
            current_page->set.in_pair[0].key 
                = parent_page->set.in_pair[k_prime_index].key;
            
            parent_page->set.in_pair[k_prime_index].key
                = neighbor_page->set.in_pair[neighbor_page->num_keys - 1].key;
            current_page->page_offset
                = neighbor_page->set.in_pair[neighbor_page->num_keys - 1].page_offset;

            memset(&neighbor_page->set.in_pair[neighbor_page->num_keys - 1], 0, 16);

            Page child_page_;
            Page *child_page;
            child_page = &child_page_;
            load_page(fp, current_page->page_offset, child_page);

            child_page->parent_or_free_page_offset = current_offset;
            fseek(fp, current_page->page_offset, SEEK_SET);
            fwrite(child_page, 1, PAGESIZE, fp);
        }

        else{
            for(i = current_page->num_keys; i > 0; i--){
                current_page->set.pair[i].key = current_page->set.pair[i - 1].key;
                strcpy(current_page->set.pair[i].value, current_page->set.pair[i - 1].value);
            }

            current_page->set.pair[0].key 
                = neighbor_page->set.pair[neighbor_page->num_keys - 1].key;
            strcpy(current_page->set.pair[0].value,
                    neighbor_page->set.pair[neighbor_page->num_keys - 1].value);

            parent_page->set.in_pair[k_prime_index].key
                = current_page->set.pair[0].key;

            memset(&neighbor_page->set.pair[neighbor_page->num_keys - 1], 0, 128);
        }

        current_page->num_keys++;
        neighbor_page->num_keys--;
    }

    fseek(fp, current_page->parent_or_free_page_offset, SEEK_SET);
    fwrite(parent_page, 1, PAGESIZE, fp);

    fseek(fp, current_offset, SEEK_SET);
    fwrite(current_page, 1, PAGESIZE, fp);

    fseek(fp, neighbor_offset, SEEK_SET);
    fwrite(neighbor_page, 1, PAGESIZE, fp);
    fflush(fp);

    return;
}

/* Deletes an entry from the page
 * Removes the key and value from leaf page,
 * and then makes all appropriate
 * changes to preserve the B+ tree properties
 */
void delete_entry(int64_t key, uint64_t offset){
    int min_keys;
    int neighbor_index;
    int k_prime_index;
    int64_t k_prime;
    int capacity;
    uint64_t neighbor_offset;

    Page current_page_;
    Page *current_page;
    current_page = &current_page_;
    remove_entry_from_page(key, offset);

    /* Case : deletion from the root */
    if(offset == headPage->root_page_offset){
        return adjust_root(offset);
    }

    /* Case : deletion from a page below the root page
     * (Rest of the function body)
     */

    /* Determine minimum allowable size of page,
     * to be preserved after deletion
     */

    load_page(fp, offset, current_page);

    min_keys = current_page->is_leaf ? cut(leaf_order) -1 : cut(internal_order) - 1;

    /* Case: page stays at or above minimum
     * (The simple case)
     */
    if(current_page->num_keys >= min_keys)
        return;

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

    Page parent_page_;
    Page *parent_page;
    parent_page = &parent_page_;
    load_page(fp, current_page->parent_or_free_page_offset, parent_page);

    neighbor_index = get_neighbor_index(offset);

    switch(neighbor_index){
        case -1:
            k_prime_index = parent_page->num_keys - 1;
            if(k_prime_index == 0)
                neighbor_offset = parent_page->page_offset;
            else
                neighbor_offset = parent_page->set.in_pair[k_prime_index - 1].page_offset;
            break;
        
        default:
            k_prime_index = neighbor_index;
            neighbor_offset = parent_page->set.in_pair[k_prime_index].page_offset;
            break;
    }
    k_prime = parent_page->set.in_pair[k_prime_index].key;

    capacity = current_page->is_leaf ? leaf_order -1  : internal_order - 1;

    Page neighbor_page_;
    Page *neighbor_page;
    neighbor_page = &neighbor_page_;
    load_page(fp, neighbor_offset, neighbor_page);

    /* Coalescence */
    if((neighbor_page->num_keys + current_page->num_keys) < capacity){
        coalesce_pages(offset, neighbor_offset, neighbor_index, k_prime);
        return;
    }

    /* Redistribution */
    else{
        redistribute_pages(offset, neighbor_offset, neighbor_index, k_prime_index, k_prime);
        return;
    }
}


/* Master deletion function */
int delete(int64_t key){
    uint64_t offset;

    if(find(key) == NULL){
        return -1;
    }
    
    offset = find_leaf_page(key);

    delete_entry(key, offset);
    return 0;
}
