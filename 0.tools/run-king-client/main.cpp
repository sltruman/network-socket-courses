#include <iostream>
#include <string>
#include <thread>
#include <list>
#include <mutex>
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

auto run_a_step = [](string server,unsigned short port,msg& req) {
    int fd = socket(AF_INET,SOCK_STREAM,0);
    struct timeval timeout = {5,0};
    setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));

    sockaddr_in svraddr;
    fill_n(reinterpret_cast<char*>(&svraddr),sizeof(svraddr),0);
    svraddr.sin_family=AF_INET;
    svraddr.sin_addr.s_addr = inet_addr(server.data());
    svraddr.sin_port=htons(port);

    auto lastSteps = req.steps;
    if(-1 == connect(fd,(sockaddr*)&svraddr,sizeof(svraddr))) goto END_RUNING;
    {
        if(-1 == send(fd,&req,sizeof(msg),MSG_NOSIGNAL)) goto END_RUNING;
        for(int j=0;j < sizeof(msg);) {
            auto len = recv(fd,reinterpret_cast<char*>(&req) + j,sizeof(msg) - j,MSG_NOSIGNAL);
            if(len == -1) goto END_RUNING;
            j += len;
        }
    }

END_RUNING:
    close(fd);

    if(req.steps == lastSteps) return false;
    return true;
};

static auto score = 0;
static mutex m;

auto runner = [](string server,unsigned short port,int i,int steps,int ms) {
    auto t = clock();
    stringstream id;
    id << server << ':' << port << ' ' << i;
    msg req = { run ,'\0',0};
    id.str().copy(req.id,id.str().size(),0);

    while(req.steps < steps) {
        run_a_step(server,port,req);
        this_thread::sleep_for(chrono::milliseconds(ms));
    }

    lock_guard<mutex> lock(m);
    score += clock() - t;
};

int main(int argc,char* argv[]) {
//    argc = 4;
//    argv[1] = "127.0.0.1";
//    argv[2] = "6000";
//    argv[3] = "100";

    if(argc < 4) {
        cout << "program <ip> <port> <num> [steps] [sleep for milliseconds]" << endl;
        return 0;
    }

    auto ip = argv[1];
    auto port = stoi(argv[2]);
    auto num = stoi(argv[3]);
    auto steps = 99999999;
    auto ms = 0;
    if(argc >= 5) steps = stoi(argv[4]);
    if(argc >= 6) ms = stoi(argv[5]);

    list<thread> ts;

    for(int i=0;i < num;i++)
        ts.emplace_back(runner,ip,port,i,steps,ms);

    for(auto& t : ts) {
        t.join();
    }

    cout << ip << ':' << port << ' ' << score << endl;

    return 0;
}
