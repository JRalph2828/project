#include <iostream>
#include <unordered_set>
#include <queue>
#include <string>
#include <map>
#include <cstring>
#include <pthread.h>

#define NUM_THREAD  34

using namespace std;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

const int SIGMA_SIZE = 26;      // Number of alphabet
multimap<pair<size_t, size_t>, string> result;  // Result map for found string
string buf;             // String for query
bool deleted = false;   // If there is deletion?
int cnt = 0;            // Which thread is next? 
int jobs = 0;           // Second thread jobs
int thread_ret[NUM_THREAD];

// Struct for thread INFO
struct ThreadInfo{
    bool trigger = true;
    bool updated = false;
    bool ready = false;
    unordered_set<string> pattern;
};
ThreadInfo tData[NUM_THREAD];

// Struct for aho-corasick struct Trie
struct Trie {
    Trie *edges[SIGMA_SIZE];
    Trie *fail;
    unordered_set<string> out;

    Trie() {
        fail = NULL;
        for (int i = 0; i < SIGMA_SIZE; i++) {
            edges[i] = NULL;
        }
    }
};

// Add string fot tree
void addString(Trie *node, Trie *root, string curString) {
    int next;
    for(int i = 0; i < curString.size(); i++){
        next = curString[i] - 'a';
        if(node->edges[next] == NULL || node->edges[next] == root)
            node->edges[next] = new Trie();
            
        node = node->edges[next];
    }
    node->out.insert(curString);
    return;
}

// Find which thread has deletion
void* FindDeletion(void* arg){
    long tid = (long)arg;
    
    // All threads will sleep here
    pthread_mutex_lock(&mutex2);
    thread_ret[tid] = -1;
    pthread_cond_wait(&cond2, &mutex2);
    pthread_mutex_unlock(&mutex2);

    while(1){
        if(tData[tid].pattern.find(buf) != tData[tid].pattern.end()){
            tData[tid].pattern.erase(buf);
            cnt = tid;
            if(tData[tid].pattern.size() == 5)
                deleted = true;
            tData[tid].updated = true;
        }

        // Thread will sleep here
        pthread_mutex_lock(&mutex2);
        jobs++;
        pthread_cond_wait(&cond2, &mutex2);
        pthread_mutex_unlock(&mutex2);
    }

    return NULL;
}

// Each thread get their string and do aho-corasick
// Afer do aho-corasick, they save result
void* ThreadFunc(void* arg){
    long tid = (long)arg;
    Trie *root;
    queue<Trie*> q;

    // All threads will sleep here
    pthread_mutex_lock(&mutex);
    tData[tid].ready = true;
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);

    while(1){
        // Make tree again
        // First time, updated, thread has no string to find
        if(tData[tid].trigger || tData[tid].updated || deleted){
            root = new Trie();
            
            // Make root
            for (int i = 0; i < SIGMA_SIZE; i++) {
                root->edges[i] = root;
            }
            
            // Make tree with string
            for(auto s : tData[tid].pattern)
                addString(root, root, s);

            root->fail = root;
            for (int i = 0; i < SIGMA_SIZE; i++) {
                if (root->edges[i] != NULL && root->edges[i] != root) {
                    root->edges[i]->fail = root;
                    q.push(root->edges[i]);
                }
            }
            // Make fail link for each node with BFS
            while (!q.empty()) {
                Trie *curNode = q.front();
                q.pop();

                for (int i = 0; i < SIGMA_SIZE; i++) {
                    Trie *next = curNode->edges[i];
                    if (next != NULL && next != root) {
                        q.push(next);

                        Trie *f = curNode->fail;
                        for (; f->edges[i] == NULL; f = f->fail);
                        next->fail = f->edges[i];
                        
                        for (auto s : next->fail->out) 
                            next->out.insert(s);
                    }
                }
            }
            tData[tid].trigger = false;
            tData[tid].updated = false;
        }

        // Make result map that has string start index, 
        // finish index and string
        int len = 0;
        unordered_set<string> temp;
        unordered_set<string>::iterator it3;
        size_t pos;
        Trie *node  = root;
        for (size_t i = 0; i < buf.size(); i++) {
            len++;
            int cur = buf[i] - 'a';
            for (; node->edges[cur] == NULL; node = node->fail);
            node = node->edges[cur];

            if (node->out.size() != 0) {
                // Get lock for make result - global variable
                pthread_mutex_lock(&mutex);
                for(auto s : node->out){
                    it3 = temp.find(s);
                    if(it3 == temp.end()){
                        pos = (size_t)(len - s.size());
                        result.insert(make_pair(make_pair(pos,i),s));
                        temp.insert(s);
                    }
                }
                pthread_mutex_unlock(&mutex);
            }
        }
        // After do task, thread fall sleep
        // until main thread send signal
        pthread_mutex_lock(&mutex);
        tData[tid].ready = true;
        pthread_cond_wait(&cond, &mutex);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

// Make threads and find string
int main() {
    unordered_set<string> word_list;
    pthread_t threads[NUM_THREAD];
    pthread_t threads2[NUM_THREAD];

    // Make first thread set for find string
    // Threads will fall sleep until get signal
    for(long i = 0; i < NUM_THREAD; i++){
        if(pthread_create(&threads[i], 0, ThreadFunc, (void*)i) < 0){
            printf("pethread_create error!\n");
            return 0;
        }
        while(!tData[i].ready) pthread_yield();
    }

    // Make second thread set for find which thread has deletion
    // Threads will fall sleep until get signal
    for(long i = 0; i < NUM_THREAD; i++){
        if(pthread_create(&threads2[i], 0, FindDeletion, (void*)i) < 0){
            printf("pethread_create error!\n");
            return 0;
        }
        while(thread_ret[i] != -1)
            pthread_yield();
    }
    jobs = 0;
    
    // Put initial string to word_list
    int nrStrings;
    cin >> nrStrings;
    for (int i = 0; i < nrStrings; i++) {
        cin >> buf;
        word_list.insert(buf);
    }
    
    // Distribute word_list for each thread
    for(auto s : word_list){
        tData[cnt].pattern.insert(s);
        cnt = (cnt+1) % NUM_THREAD;
    }

    cout << "R" << endl;
    char cmd;
    bool isString = false;
    while(cin >> cmd){
        cin.get();
        getline(cin,buf);
        switch(cmd){
            case 'Q':
                {
                    // If thread has no word to find
                    // make thread pattern again
                    if(deleted){
                        cnt = 0;
                        for(int i = 0; i < NUM_THREAD ; i++)
                            tData[i].pattern.clear();

                        for(auto s : word_list){
                            tData[cnt].pattern.insert(s);
                            cnt = (cnt+1) % NUM_THREAD;
                        }
                    }
                    for(int i = 0; i < NUM_THREAD; i++)
                        tData[i].ready = false;
                    
                    // Wake up all threads sleeping with cond
                    // (First thread set)
                    pthread_mutex_lock(&mutex);
                    pthread_cond_broadcast(&cond);
                    pthread_mutex_unlock(&mutex);

                    // Master thread will wait here untill all threads done
                    for(int i = 0; i < NUM_THREAD; i++)
                        while(!tData[i].ready) pthread_yield();
                    deleted = false;

                    // Print out result
                    multimap<pair<size_t, size_t>, string>::iterator it;
                    it = result.begin();
                    for(int i = result.size(); i != 0; i--,it++){
                        cout << it->second;
                        if(i != 1) cout << "|";
                        isString = true;
                    }
                    if(!isString) cout << -1;
                    cout << std::endl;
                    isString = false;
                    result.clear();
                }
                break;
            case 'A':
                {
                    // If duplicated string come, then break
                    if(word_list.find(buf) != word_list.end()) break;

                    // Else put string to word_list and thread pattern
                    // Set next thread who will get string
                    word_list.insert(buf);
                    tData[cnt].pattern.insert(buf);
                    tData[cnt].updated = true;
                    cnt = (cnt+1) % NUM_THREAD;
                    break;
                }
            case 'D':
                {
                    // If duplicated string come, then break;
                    if(word_list.find(buf) == word_list.end()) break;
                    // Else remove string from word_list
                    word_list.erase(buf);
                    // Wake up sleeping thread with cond2
                    // (thread set 2)
                    pthread_mutex_lock(&mutex2);
                    pthread_cond_broadcast(&cond2);
                    pthread_mutex_unlock(&mutex2);
                    
                    // Master thread will wait here untill all threads done
                    while(1){
                        if(jobs == NUM_THREAD)
                            break;
                        pthread_yield();
                    }
                    jobs = 0;
                    break;
                }
        }
    }

    return 0;
}

