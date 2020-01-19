#ifndef SERVEROFBASIC_H
#define SERVEROFBASIC_H

#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <mutex>
#include <thread>
using namespace std;

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/poll.h>
#include <netinet/in.h>

namespace of_basic {
    struct msg {
        int flag;
        char id[24];
        unsigned int steps;
    };

    static mutex m;
    static map<string,unsigned int> status;

    void task(int fd) {
         msg req;
BEGIN:
        for(int j=0;j < sizeof(req);) {
            auto len = recv(fd,reinterpret_cast<char*>(&req) + j,sizeof(req) - j,MSG_NOSIGNAL);
            if(len <= 0) goto RET;
            j += len;
        }

        switch(req.flag) {
        case 0:{
            lock_guard<mutex> lock(m);
            cout << "run:" << req.steps << endl;
            req.steps++;
            status[req.id] = req.steps;
            if(-1 == send(fd,&req,sizeof(req),MSG_NOSIGNAL)) goto RET;
            break;}
        default:{
            msg res;
            lock_guard<mutex> lock(m);
            auto runner_num = status.size();
            cout << "peek:" << runner_num << endl;

            if(-1 == send(fd,&runner_num,4,MSG_NOSIGNAL)) goto RET;
            for(auto& kv : status) {
                fill_n(res.id,24,0);
                kv.first.copy(res.id,kv.first.size(),0);
                res.steps = kv.second;
                if(-1 == send(fd,&res,sizeof(res),MSG_NOSIGNAL)) goto RET;
                cout << res.id << ' ' << res.steps << endl;
            }

            goto BEGIN;}
        }

        RET:
        cout << "close:" << fd << endl;
        close(fd);
    }

    void server(unsigned short port) {
        int fd_self = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        int tmp = 1;
        setsockopt(fd_self, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(int));

        sockaddr_in addr_s,addr_c;
        socklen_t addr_c_len = sizeof(sockaddr_in);
        addr_s.sin_family = AF_INET;
        addr_s.sin_port = htons(port);
        addr_s.sin_addr.s_addr = INADDR_ANY;

        if(-1 == bind(fd_self, reinterpret_cast<sockaddr*>(&addr_s), sizeof(sockaddr))) goto RET;
        if(-1 == listen(fd_self, 1024)) goto RET;

        while(true) {
            int fd_new = accept(fd_self,reinterpret_cast<sockaddr*>(&addr_c),&addr_c_len);
            if(fd_new == -1) {
                cout << "accept:" << errno << endl;
                goto RET;
            }
            cout << "new:" << fd_new << endl;
            thread(task,fd_new).detach();
        }

        RET:
        close(fd_self);
    }
}

#endif // SERVEROFBASIC_H
