#include <iostream>
#include "server-of-select.h"
#include "server-of-poll.h"

using namespace std;

int main()
{
    of_select::server(6000);
    //of_poll::server(6001);
    return 0;
}
