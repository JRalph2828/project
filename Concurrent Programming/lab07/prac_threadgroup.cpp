#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

# define NUM_THREAD_IN_POOL 4

using namespace std;
boost::asio::io_service io2;

void Print(int start_range, int end_range, int result, int idx) {
    std::cout << "(" << idx <<  ")" << "number if primes in " << start_range << " ~ " <<
       end_range << " is " << result << std::endl;
}

bool IsPrime(int n) {
    if (n < 2) {
        return false;
    }

    for (int i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

void findPrime(int start_range, int end_range, int idx){
    int cnt_prime = 0;
    for(int i = start_range; i <= end_range; i++){
        if(IsPrime(i))
            cnt_prime++;
    }
    io2.post(boost::bind(Print, start_range, end_range, cnt_prime, idx));
}

int main(void) {
    int start_range;
    int end_range;
    int result;
    int idx = 0;

    boost::asio::io_service io;
    boost::thread_group threadpool;
    boost::asio::io_service::work* work = new boost::asio::io_service::work(io);
    boost::asio::io_service::work* work2 = new boost::asio::io_service::work(io2);

    boost::thread thd(boost::bind(&boost::asio::io_service::run, &io2));

    for (int i = 0; i < NUM_THREAD_IN_POOL; i++) {
        threadpool.create_thread(boost::bind(
                    &boost::asio::io_service::run, &io));
    }

    while (1) {
        cin >> start_range;
        if(start_range == -1)
            break;
        cin >> end_range;
        idx++;

        io.post(boost::bind(findPrime, start_range, end_range, idx));
    }

    delete work;
    threadpool.join_all();
    delete work2;
    io2.stop();
    io.stop();

    return 0;
}
