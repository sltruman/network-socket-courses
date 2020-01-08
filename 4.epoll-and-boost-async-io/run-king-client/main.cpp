#include <iostream>
#include <string>
#include <thread>
#include <sstream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;

enum msg_type { run };

struct msg {
    msg_type flag;
    char id[24];
    unsigned int steps;
};

static bool running = true;

auto run_a_step = [](string server,unsigned short port,msg& buff) {
    int fd = socket(AF_INET,SOCK_STREAM,0);
    struct timeval timeout = {10,0};
    setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));

    sockaddr_in svraddr;
    fill_n(reinterpret_cast<char*>(&svraddr),sizeof(svraddr),0);
    svraddr.sin_family=AF_INET;
    svraddr.sin_addr.s_addr = inet_addr(server.data());
    svraddr.sin_port=htons(port);

    auto lastSteps = buff.steps;
    if(-1 == connect(fd,(sockaddr*)&svraddr,sizeof(svraddr))) goto END_RUNING;
    {
        if(-1 == send(fd,&buff,sizeof(msg),MSG_NOSIGNAL)) goto END_RUNING;
        for(int j=0;j < sizeof(msg);) {
            auto len = recv(fd,reinterpret_cast<char*>(&buff) + j,sizeof(msg) - j,MSG_NOSIGNAL);
            if(len == -1) goto END_RUNING;
            j += len;
        }
    }

END_RUNING:
    close(fd);

    if(buff.steps == lastSteps) return false;
    return true;
};

auto runner = [](string server,unsigned short port,int i) {
    stringstream id;
    id << "runner:" << i;
    msg buff = { run ,'\0',0};
    id.str().copy(buff.id,id.str().size(),0);

    while(running) {
        if(!run_a_step(server,port,buff)) cout << "timeout:" << buff.id << " steps:" << buff.steps << endl;

    }
};

int main(int argc,char* argv[])
{
//    argc = 4;
//    argv[1] = "127.0.0.1";
//    argv[2] = "6000";
//    argv[3] = "100";

    if(argc != 4) {
        cout << "program <ip> <port> <num>" << endl;
        return 0;
    }

    auto ip = argv[1];
    auto port = stoi(argv[2]);
    auto num = stoi(argv[3]);

    for(int i=0;i < num;i++) {
        thread(runner,ip,port,i).detach();
    }

    cout << "press any key exit." << endl;
    getchar();
    running = false;

    return 0;
}
