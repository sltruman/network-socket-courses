#include <iostream>
#include "server-of-select.h"
#include "server-of-poll.h"
#include "server-of-epoll.h"
#include "server-of-asio.h"

using namespace std;

int main()
{
    of_select::server(6000);


    return 0;
}
