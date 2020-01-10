#ifndef SERVEROFTHREADPOOL_H
#define SERVEROFTHREADPOOL_H

#include <iostream>
#include <thread>
#include <functional>
#include <list>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <functional>

using namespace std;

class thread_pool {
public:
    thread_pool(int threads):quit(false) {
        for(int i = 0; i < threads; ++i)
            workers.emplace_back([this]{
                for(;;) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock,[this]{ return quit || !tasks.empty(); });
                        if(quit && tasks.empty()) return;

                        task = std::move(tasks.front());
                        this->tasks.pop();
                    }

                    task();
                }
            });
    }

    void post(function<void()> f) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace(f);
        condition.notify_one();                          
    }

    ~thread_pool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            quit = true;
        }

        condition.notify_all();
        for(std::thread &worker: workers) worker.join();
    }

private:
    vector< thread > workers;           
    queue< function<void()> > tasks;  
    mutex queue_mutex;                       
    condition_variable condition;        
    bool quit;                                 
};

int main() {
	const unsigned short thread_num = 50;
	auto threads_sum = 0, thread_pool_sum = 0;
	auto tick = 0;
	mutex m;

	thread threads[thread_num];
	cout << "线程组：";

	tick = clock();
	for (int i = 0; i < thread_num; i++) {
		threads[i] = thread([&, i]() {
			for (int j = 0; j < 1000000; j++) {}
			auto score = clock() - tick;
			threads_sum += score;
			lock_guard<mutex> lock(m);
			cout << score << ' ';
		});
	}

	for (int i = 0; i < thread_num; i++) {
		if (threads[i].joinable()) threads[i].join();
	}

	{
		thread_pool thread_group(thread_num);

		cout << endl << "线程池：";
		tick = clock();
		for (int i = 0; i < thread_num; i++) {
			thread_group.post([&, i]() {
				for (int j = 0; j < 1000000; j++) {}
				auto score = clock() - tick;
				thread_pool_sum += score;
				lock_guard<mutex> lock(m);
				cout << score << ' ';
			});
		}
	}

	cout << endl << "线程组总分：" << threads_sum << endl;
	cout << "线程池总分：" << thread_pool_sum << endl;

	return 0;
}

