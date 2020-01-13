#ifndef SERVEROFPOLL_H
#define SERVEROFPOLL_H

#include <iostream>
#include <vector>
#include <map>
using namespace std;

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/poll.h>
#include <netinet/in.h>

namespace of_poll {
    static struct msg {
        int flag;
        char id[24];
        unsigned int steps;
    } res;

    static map<string,unsigned int> status;
    static map<int,pair<msg,unsigned int>> requests;

    bool task(int fd) {
        auto& kv = requests[fd];
        auto& req = kv.first;
        auto& offset = kv.second;

        auto len = recv(fd,reinterpret_cast<char*>(&req) + offset,sizeof(req) - offset,0);
        if(len == -1) goto RET;
        if(len == 0) goto RET;

        offset += len;
        if(offset == sizeof(req)) offset = 0;
        else return false;

        switch(req.flag) {
        case 0:{
            cout << "run:" << endl;
            req.steps++;
            status[req.id] = req.steps;
            if(-1 == send(fd,&req,sizeof(req),0)) goto RET;
            break;}
        default:{
            auto runner_num = status.size();
            cout << "peek:" << runner_num << endl;

            if(-1 == send(fd,&runner_num,4,0)) goto RET;
            for(auto& kv : status) {
                fill_n(res.id,24,0);
                kv.first.copy(res.id,kv.first.size(),0);
                res.steps = kv.second;
                if(-1 == send(fd,&res,sizeof(res),0)) goto RET;
                cout << res.id << ' ' << res.steps << endl;
            }

            return false;}
        }

        RET:
        requests.erase(fd);
        return true;
    }

    void server(unsigned short port) {
        vector<pollfd> fds;

        int fd_self = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        pollfd fd = {fd_self,POLL_IN,0};
        fds.push_back(fd);

        int tmp = 1;
        setsockopt(fd_self, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(int));

        sockaddr_in addr_s,addr_c;
        socklen_t addr_c_len = sizeof(sockaddr_in);
        addr_s.sin_family = AF_INET;
        addr_s.sin_port = htons(port);
        addr_s.sin_addr.s_addr = INADDR_ANY;

        if(-1 == bind(fd_self, reinterpret_cast<sockaddr*>(&addr_s), sizeof(sockaddr))) goto RET;
        if(-1 == listen(fd_self, 1024)) goto RET;

        for(int fd_max = fd_self;;) {
            auto num = poll(fds.data(),fds.size(),-1);
            if(-1 == num) goto RET;

            for(auto it=fds.begin();it != fds.end() && num > 0;it++,num--) {
                if(0 == it->revents) continue;
                it->revents = 0;
                if(it->fd == fd_self) {
                    auto fd_new = accept(fd_self, reinterpret_cast<sockaddr*>(&addr_c), &addr_c_len);
                    if(fd_new == -1) goto RET;
                    cout << "new:" << fd_new << endl;
                    fd_max = max(fd_new,fd_max);
                    fd = {fd_new,POLL_IN,0};
                    fds.push_back(fd);
                    continue;
                }

                cout << "active:" << it->fd << endl;
                if(!task(it->fd)) continue;
                close(it->fd);
                cout << "close:" << it->fd << endl;
                it = --fds.erase(it);
            }
        }

        RET:
        close(fd_self);
    }
}

#endif // SERVEROFSELECT_H
