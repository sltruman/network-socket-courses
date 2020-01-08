#ifndef SERVEROFSELECT_H
#define SERVEROFSELECT_H

#include <iostream>
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
#include <sys/select.h>
#include <netinet/in.h>

namespace of_select {
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
        list<int> fds,fds_removed;
        int fd_self = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        fds.push_back(fd_self);

        int tmp = 1;
        setsockopt(fd_self, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(int));

        sockaddr_in addr_s,addr_c;
        socklen_t addr_c_len = sizeof(sockaddr_in);
        addr_s.sin_family = AF_INET;
        addr_s.sin_port = htons(port);
        addr_s.sin_addr.s_addr = INADDR_ANY;

        if(-1 == bind(fd_self, reinterpret_cast<sockaddr*>(&addr_s), sizeof(sockaddr))) goto RET;
        if(-1 == listen(fd_self, 1024)) goto RET;

        fd_set fds_readable;

        while(true) {
            FD_ZERO(&fds_readable);

            int fd_max = 0;
            for(auto fd : fds) {
                FD_SET(fd,&fds_readable);
                fd_max = max(fd,fd_max);
            }

            if(-1 == select(fd_max + 1,&fds_readable,nullptr,nullptr,nullptr)) goto RET;

            for(auto it=fds.begin();it != fds.end();it++) {
                auto fd = *it;
                if(!FD_ISSET(fd,&fds_readable)) continue;
                if(fd == fd_self) {
                    auto fd = accept(fd_self, reinterpret_cast<sockaddr*>(&addr_c), &addr_c_len);
                    if(fd == -1) goto RET;
                    cout << "new:" << fd << endl;
                    fds.push_back(fd);
                    continue;
                }

                cout << "active:" << fd << endl;
                if(!task(fd)) continue;
                close(fd);
                it = --fds.erase(it);
            }
        }

        RET:
        close(fd_self);
    }
}

#endif // SERVEROFSELECT_H
