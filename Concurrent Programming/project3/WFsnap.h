#include <stdio.h>
#include <vector>
#include <iostream>
#include <pthread.h>
#include <unistd.h>

using namespace std;
int size = 0;

class StampedSnap{
    public :
        long stamp;
        int value;
        vector<int> snap;

        StampedSnap();
        StampedSnap(long label, int value, vector<int> snap);
};

StampedSnap::StampedSnap(){
    stamp = 0;
    value = 0;
}

StampedSnap::StampedSnap(long stamp, int value, vector<int> snap){
    this->stamp = stamp;
    this->value = value;
    this->snap = snap;
}

class WFSnapshot{
    public :
        StampedSnap* a_table;

        StampedSnap* collect(void){
            StampedSnap *copy =(StampedSnap*)calloc(size, sizeof(StampedSnap));
            for(int i = 0; i < size;i++)
                copy[i] = a_table[i];
            return copy;
        };
        
        
        WFSnapshot(int capacity, int init);

        vector<int> scan(void){
            StampedSnap* old_copy;
            StampedSnap* new_copy;
            vector<bool> moved(size);

            old_copy = collect();

            while(1){
                new_copy = collect();
                vector<int> result(size);

                for(int i = 0; i < size; i++){
                    if(old_copy[i].stamp != new_copy[i].stamp){
                        if(moved[i]){
                            for(int j = 0; j < size; j ++){
                                result[j] = old_copy[i].snap[j];
                            }
                            return result;
                        }
                        else{
                            moved[i] = true;
                            old_copy = new_copy;
                            continue;
                        }
                    }
                }

                for(int i = 0; i < size; i++)
                    result[i] = new_copy[i].value;
                return result;
            }
        };

        void update(int value, long tid){
            long me = tid;
            vector<int> snap = scan();
            StampedSnap old_value = a_table[me];
            StampedSnap new_value(old_value.stamp +1, value, snap);
            a_table[me] = new_value;
        };
}; 


WFSnapshot::WFSnapshot(int capacity, int init){
    a_table =  new StampedSnap[capacity];
    for(int i = 0; i < capacity; i++){
        a_table[i].value = init;
    }
    size = capacity;
}

