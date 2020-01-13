#include <iostream>
#include <thread>

#include "server-of-select.h"
#include "server-of-poll.h"
#include "server-of-epoll.h"
#include "server-of-asio.h"

using namespace std;

int main()
{
    thread(of_select::server,6000).detach();
    thread(of_poll::server,6001).detach();
    thread(of_epoll::server,6002).detach();
    thread(of_asio::server,6003).detach();

    getchar();
    return 0;
}

