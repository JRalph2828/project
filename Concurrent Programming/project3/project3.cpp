#include "WFsnap.h"

using namespace std;

void* ThreadFunc(void*);
WFSnapshot *WF;
bool is_done = false;

int main(int argc, char * argv[]){
    int NUM_THREAD;
    NUM_THREAD = atoi(argv[1]);
    pthread_t threads[NUM_THREAD];
    
    WF = new WFSnapshot(NUM_THREAD, 0);

    for(long i = 0; i < NUM_THREAD; i++){
        if(pthread_create(&threads[i], 0, ThreadFunc, (void*)i) < 0){
            cout << "pthread_create error!" << endl;
            return 0;
        }
    }

    sleep(60);
    is_done = true;

    long ret;
    for(int i = 0; i < NUM_THREAD; i++)
        pthread_join(threads[i], (void**)&ret);

    long result = 0;
    for(int i = 0; i < NUM_THREAD; i++)
        result += WF->a_table[i].stamp;

    printf("update : (%ld)\n", result);

    return 0;
}

void* ThreadFunc(void*arg){
    long tid = (long)arg;

    while(!is_done)
        WF->update(1, tid);

}

