#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <map>
#include <vector>

#include <csignal>

#ifdef _WIN32
#include <winsock.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#endif

using namespace std;

enum msg_type { run,peek };

struct msg {
    msg_type flag;
    char id[24];
    unsigned int steps;
};

static mutex m;
static map<string,unsigned int> status;

auto task = [](int fd) {
    msg buff;

BEGIN:
    for(int j=0;j < sizeof(msg);) {
        auto len = recv(fd,&buff + j,sizeof(msg) - j,0);
        if(len == -1) goto END_TASK;
        j += len;
    }
    switch(buff.flag) {
    case msg_type::run:{
        lock_guard<mutex> lock(m);
        buff.steps++;
        status[buff.id] = buff.steps;
        if(-1 == send(fd,&buff,sizeof(msg),MSG_NOSIGNAL)) goto END_TASK;
        break;}
    default:{
        lock_guard<mutex> lock(m);
        auto runner_num = status.size();
        cout << "peek:" << runner_num << endl;

        msg r;

        if(-1 == send(fd,reinterpret_cast<char*>(&runner_num),4,MSG_NOSIGNAL)) goto END_TASK;
        for(auto& kv : status) {
            fill_n(r.id,24,0);
            kv.first.copy(r.id,kv.first.size(),0);
            r.steps = kv.second;
            if(-1 == send(fd,&r,sizeof(msg),MSG_NOSIGNAL)) goto END_TASK;
            cout << r.id << ' ' << r.steps << endl;
        }

        goto BEGIN;}
    }

END_TASK:
    close(fd);
};

static int sockfd ;
static bool running = true;

void signal_handler(int s) {
    if (s != SIGTERM) return;
    running = false;
    close(sockfd);
}

int main() {

#ifdef _WIN32
  WSADATA wsaData;
  if(-1 == WSAStartup(MAKEWORD(2,2), &wsaData)) return 1;
#endif

    sockfd = socket(AF_INET,SOCK_STREAM,0);
    int tmp = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(int));

    sockaddr_in svraddr;
    fill_n(reinterpret_cast<char*>(&svraddr),sizeof(svraddr),0);
    svraddr.sin_family = AF_INET;
    svraddr.sin_addr.s_addr = INADDR_ANY;
    svraddr.sin_port=htons(6000);

    if(-1 == bind(sockfd,reinterpret_cast<sockaddr*>(&svraddr),sizeof(svraddr))) goto END;
    if(-1 == listen(sockfd,1024)) goto END;

    signal(SIGTERM, signal_handler);

    for(unsigned int i=0;running;i++) {
        sockaddr_in clientaddr;
        socklen_t addr_len = sizeof(clientaddr);
        int fd = accept(sockfd,reinterpret_cast<sockaddr*>(&clientaddr),&addr_len);
        if(fd == -1) goto EXIT;
        thread(task,fd).detach();
    }

END:
    close(sockfd);
    cout << "error code:" << errno << endl;

EXIT:
    cout << "exit" << endl;


#ifdef _WIN32
    WSACleanup();
#endif


    return 0;
}
