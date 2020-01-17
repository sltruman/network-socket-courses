#include <iostream>
#include <ctime>

using namespace std;

template <typename T>
class pool {
    struct block {
        T data;
        block* next;
    };

public:
    pool(unsigned int max_num) {
        idle_buffer = nullptr;

        for (int i = 0; i < max_num; i++) {
            auto buffer = new block;
            fill_n((char*)buffer, sizeof(block), 0);
            buffer->next = idle_buffer;
            idle_buffer = buffer;
        }
    }

    ~pool() {
        while(idle_buffer == nullptr) {
            auto buffer = idle_buffer->next;
            delete idle_buffer;
            idle_buffer = buffer;
        }
    }

    T* malloc() {
        auto p = reinterpret_cast<T*>(idle_buffer);
        if(p != nullptr) idle_buffer = idle_buffer->next;
        return p;
    }

    void free(T* p) {
        idle_buffer = reinterpret_cast<block*>(p)->next = idle_buffer;
    }

private:
    block *idle_buffer;
};

int main()
{
    struct test {
        char b[64];
        int bb[64];
        double bbb[64];
    };

    const int max_num = 5000000;
    pool<test> pl(max_num);

    auto t = clock();
    for (int i = 0; i < max_num; i++)
    {
        auto p = new test();
        delete p;
    }
    cout << clock() - t << endl;

    t = clock();
    for (int i = 0; i < max_num; i++)
    {
        auto p = pl.malloc();
        new (p)test();
        p->~test();
        pl.free(p);
    }
    cout << clock() - t << endl;

    return 0;
}

