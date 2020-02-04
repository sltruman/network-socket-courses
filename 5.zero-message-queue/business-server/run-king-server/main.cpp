#include <iostream>
#include <thread>

#include "server-of-asio.h"

using namespace std;

int main()
{
    thread(of_asio::server,6004).detach();

    getchar();
    return 0;
}

