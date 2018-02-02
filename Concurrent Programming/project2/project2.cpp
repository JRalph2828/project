#include <iostream>
#include <fstream>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <set>
#include <stack>

using namespace std;

// Global mutex & condition
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int E = 0;          // Execution number
int GEO = 0;        // Global execution order
int NUM_THREAD = 0; // Number of threads
int NUM_RECORD = 0; // Number of records

// Thread node
typedef struct _node{
    struct _node* next; // Next node
    struct _node* prev; // Previous node
    pthread_cond_t myCond;  // Thread's condition
    int tid;
    bool isReader;
}node;

// Record struct with R&W lock
struct rwlock {
    // Number of readers, writers, waiting readers, waiting writers
    unsigned readers, writers, read_waiters, write_waiters;
    
    // Node pointer for each linked list's head
    node* head;     
    long long int result; // Record's result;
    int rid;    // Record's id;
    set<int> own_tid;       // Lock holder's tid
    set<int> wait_list_tid; // Wait lock's tid
    bool isReader;          // Reader lock or writer lock?
};
// Record vector
vector<rwlock*> record; 

/* Functions */
int search_list(int);
bool dead_lock_check(int, int);
void roll_back(node*, int);
void insert_node(rwlock*, bool, int);
void delete_node(node*);
void reader_lock(rwlock*, int);
void reader_unlock(rwlock*, int);
bool writer_lock(rwlock*, int);
void writer_unlock(rwlock*, int);
void init_wrlock(rwlock*, int);
int get_rand_num(void);
void undo(int, int, int);
void* ThreadFunc(void*);


// Find record id that contains lock holder we want to find
int search_list(int target_tid){
    set<int>::iterator it;

    // Which record target_tid exist?
    int rid;
    bool find = false; // Find?

    // Search record's wait_list_tid
    // if there is target tid
    for(rid = 0; rid < NUM_RECORD; rid++){
        it = record[rid]->wait_list_tid.find(target_tid);

        // If find
        if(it != record[rid]->wait_list_tid.end()){
            find = true;
            break;
        }
    }
    
    // Find return record id, else -1
    if(find)
        return rid;
    else
        return -1;
}

// Check for Dead Lock
bool dead_lock_check(int tid, int rid){
    // Target tid & rid with initialization
    int target_tid = -1;
    int target_rid = -1;

    // Iterator for check tid is exists
    set<int>::iterator it;

    // Stack for save target tid
    stack<int> s;

    // Initialize stack with added node's record value
    for(auto c : record[rid]->own_tid)
        s.push(c);

    // Until stak will be empty
    while(!s.empty()){
        // Save target tid and pop
        target_tid = s.top();
        s.pop();

        // Search target tid's rid
        target_rid = search_list(target_tid);

        // Only target tid is in linked list go here
        // if fail to find target tid in linked list, just go back loop
        if(target_rid != -1){
            it = record[target_rid]->own_tid.find(tid);

            // If there is a lock holder in record same as tid
            // then we find Dead Lock
            if(it != record[target_rid]->own_tid.end())
                return true;
            
            // Else just push next target tid
            for(auto c : record[target_rid]->own_tid)
                s.push(c);
        }
    }

    return false;
}

// Roll back when Dead Lock is detected
void roll_back(node *pos, int tid){
    int target_rid[2] = {-1, -1};

    set<int>::iterator it;
   
    // Delete lastly add node 
    delete_node(pos);

    // Roll back state 
    for(int rid = 0; rid < NUM_RECORD; rid++){
        it = record[rid]->own_tid.find(tid);

        // If record has same tid lock holder
        if(it != record[rid]->own_tid.end()){

            // Reader lock go here
            if(record[rid]->isReader){
                // Unlock reader lock and save reader lock rid
                reader_unlock(record[rid], tid);
                target_rid[0] = rid;
            }

            // Writer lock go here
            else{
                // Unlock writer lock and save write lock rid
                writer_unlock(record[rid], tid);
                target_rid[1] = rid;
            }
        }
    }

    // Roll back first write result 
    // when Dead Lock caused by second writer lock
    if(target_rid[1] != -1)
        record[target_rid[1]]->result -= record[target_rid[0]]->result+1;
}

// Insert new node
void insert_node(rwlock* self, bool isReader, int tid){
    // Allocate new node
    node *newNode = (node*)malloc(sizeof(node));

    // Set temp node pointer
    node *p = self->head;

    // Initailize new node's info
    newNode->next = NULL;
    newNode->tid = tid;
    newNode->isReader = true;
    pthread_cond_init(&newNode->myCond,NULL);

    // Find tail and set link
    while(p->next != NULL) p = p->next ;
    p->next = newNode;
    newNode->prev = p;

    // Set reader lock or writer lock
    if(isReader)
        newNode->isReader = true;
    else
        newNode->isReader = false;
}

// Delete completed node
void delete_node(node *target){
    // Reset link of prev node
    target->prev->next = target->next;
    
    // Reset link of next node
    if(target->next != NULL)
        target->next->prev = target->prev;

    // Free node struct that created by malloc
    free(target);
}

void reader_lock(struct rwlock *self, int tid) {
    // Insert linked list
    insert_node(self, true, tid);

    // Set node's position and
    // count number of waiting writer in fornt of me
    int pre_wait_writer = 0;
    node *pos = self->head;
    while(pos->tid != tid){
        if(!pos->isReader)
            pre_wait_writer++;
        pos = pos->next;
    }
    
    // If write lock holder or pre_wait_writer exists
    // go here and wait
    if(self->writers || pre_wait_writer){
        // Increase reader lock waiters
        self->read_waiters++;

        // Insert current tid to wait list
        self->wait_list_tid.insert(tid);

        while(1){
            pthread_cond_wait(&pos->myCond, &mutex);

            // Is there pre_wait_writer exists?            
            pre_wait_writer = 0;
            node *temp = self->head;
            while(temp != pos){
                if(!temp->isReader)
                    pre_wait_writer++;
                temp = temp->next;
            }
            
            // No more write lock holders 
            // and no more pre_wait_writer 
            // then get out of loop
            if(self->writers == 0 && pre_wait_writer == 0)
                break;
        }
        // Decrease reader lock waiters
        self->read_waiters--;
    }
    
    // Remove from linked list
    delete_node(pos);

    // Remove from wait list
    self->wait_list_tid.erase(tid);

    // Save current lock holder's tid
    self->own_tid.insert(tid);

    // Set current lock is reader lock
    self->isReader = true; 

    // Increase number of read lock holders
    self->readers++;

}

void reader_unlock(struct rwlock *self, int tid) {
    // Decrease number of read lock holders
    self->readers--;

    // Erase current lock holder's tid
    self->own_tid.erase(tid);

    // Linked list is not empty and no more reader lock holders exist
    // then first node will be writer lock waiter
    if(self->head->next != NULL && self->readers == 0)
        pthread_cond_signal(&self->head->next->myCond);
}

bool writer_lock(struct rwlock *self, int tid) {
    insert_node(self, false, tid);
    
    // Count how many node infront of me
    int pre_waiter = 0;
    node *pos = self->head->next;
    while(pos->tid != tid){
        pos = pos->next;
        pre_waiter++;
    }

    // If reader lock holder or wirter lock holder
    // or pre_waiter exists, go here and wait
    if(self->readers || self->writers || pre_waiter){
        // Check for Dead Lock
        if(dead_lock_check(tid, self->rid)){
            // If Dead Lock exsits, roll back and return
            roll_back(pos, tid);
            return true;
        }

        self->write_waiters++;
        self->wait_list_tid.insert(tid);
        
        while(1){
            pthread_cond_wait(&pos->myCond, &mutex);

            // Is there ant node infront of me?
            pre_waiter = 0;
            node *temp = self->head->next;
            while(temp != pos){
                pre_waiter++;
                temp = temp->next;
            }
            // No more reader lock holder and
            // no more writer lock holder and
            // no more nodes infront of me
            // then get out of loop
            if(self->readers == 0 && self->writers == 0 && pre_waiter == 0)
                break;
        }
        self->write_waiters--;
    }

    delete_node(pos);
    self->wait_list_tid.erase(tid);
    self->own_tid.insert(tid);
    self->isReader = false;
    self->writers++;

    return false;
}

void writer_unlock(struct rwlock *self, int tid) {
    self->writers--;
    self->own_tid.erase(tid);

    // Linked list is not empty and first node is writer, go here
    if(self->head->next != NULL && self->head->next->isReader == false)
        pthread_cond_signal(&self->head->next->myCond);

    // Linked list is not empty
    else if(self->head->next != NULL){
        // First node will be reader
        node *temp = self->head;

        // Until writer waiter comes
        // or linked list is empty
        // wake up all readers
        while(1){
            if(temp->next == NULL) break;

            temp = temp->next;

            if(temp->isReader)
                pthread_cond_signal(&temp->myCond);
            else
                break;
        }
    }
}

// Initialize R&W lock
void init_rwlock(struct rwlock *self, int i) {
    self->readers = 0;
    self->writers = 0;
    self->read_waiters = 0;
    self->write_waiters = 0;
    self->result = 100;
    self->rid = i;
    self->isReader = true;
    self->head->next = NULL;
    self->head->prev = NULL;
    self->head->tid = -1;
    self->head->isReader = true;
}

// Get random number
int *get_rand_num(int n){
    int *randNum = (int*)malloc(n);
    
    // Get first random number
    randNum[0] = rand() % NUM_RECORD;

    // Get second random number iteratively until 
    // get unique value
    do randNum[1] = rand() % NUM_RECORD;
    while(randNum[1] == randNum[0]);

    // Get third random number iteratively until
    // get unique value
    do randNum[2] = rand() % NUM_RECORD;
    while(randNum[2] == randNum[0] || randNum[2] == randNum[1]);

    return randNum;
}

void Undo(int j, int k, long long int temp){
    record[j]->result -= temp + 1;
    record[k]->result += temp;
}

// Thread function
void* ThreadFunc(void*arg){
    long tid = (long)arg;
    string s = "thread" + to_string(tid+1) + ".txt";
    bool isDeadLock = false;

    ofstream out;
    out.open(s);

    while(1){
        int i, j, k, commit_id;
        long long int temp;
        string result;

        int *rand = get_rand_num(3);
        i = rand[0];
        j = rand[1];
        k = rand[2];

        /* 2PL growing phase */
        pthread_mutex_lock(&mutex);
        reader_lock(record[i], tid);
        pthread_mutex_unlock(&mutex);
        temp = record[i]->result;

        pthread_mutex_lock(&mutex);
        isDeadLock = writer_lock(record[j], tid);
        pthread_mutex_unlock(&mutex);
        
        // If Dead Lock detected go back loop
        if(isDeadLock)
            continue;
        record[j]->result += temp + 1;

        pthread_mutex_lock(&mutex);
        isDeadLock = writer_lock(record[k], tid);
        pthread_mutex_unlock(&mutex);

        // If Dead Lock detected go back loop
        if(isDeadLock)
            continue;
        record[k]->result -= temp;

        /* 2PL shrinking phase */
        pthread_mutex_lock(&mutex);
        reader_unlock(record[i], tid);
        writer_unlock(record[j], tid);
        writer_unlock(record[k], tid);

        // Increment global execution
        GEO++;
        commit_id = GEO;

        // If commit_id is bigger than execution number
        // Undo transaction and set isDone FLAG as true
        if(commit_id > E){
            Undo(j, k, temp);
            out.close();
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Save result and append commit log
        result = to_string(commit_id) + " " + to_string(i) + " " + to_string(j) + " " + 
            to_string(k) + " " + to_string(record[i]->result) + " " + 
            to_string(record[j]->result) + " " + to_string(record[k]->result) + "\n";

        out << result;

        pthread_mutex_unlock(&mutex);

    }
    out.close();
    return NULL;
}

int main(int argc, char * argv[]){
    NUM_THREAD = atoi(argv[1]);
    NUM_RECORD = atoi(argv[2]);
    E = atoi(argv[3]);
    srand(time(NULL));
    pthread_t threads[NUM_THREAD];
    record.reserve(NUM_RECORD);

    // Initailize linked lists
    for(int i = 0; i < NUM_RECORD; i++){
        record[i] = new rwlock();
        record[i]->head = new _node();
        init_rwlock(record[i], i);
    }

    for(long i = 0; i < NUM_THREAD; i++){
        if(pthread_create(&threads[i], 0, ThreadFunc, (void*)i) < 0){
            cout << "pthread_create error!" << endl;
            return 0;
        }
    }

    // Master thread is waiting here
    long ret;
    for(int i = 0; i < NUM_THREAD; i++)
        pthread_join(threads[i], (void**)&ret);
    
    // Delete allocated vector
    for(int i = 0; i < NUM_RECORD; i++){
       delete record[i]->head;
       delete record[i];
    }

    return 0;
}
