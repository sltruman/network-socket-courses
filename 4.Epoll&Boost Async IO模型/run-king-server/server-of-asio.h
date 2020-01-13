#ifndef SERVEROFASIO_H
#define SERVEROFASIO_H

#include <iostream>
#include <map>
#include <memory>
using namespace std;

#include <boost/asio.hpp>

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

namespace of_asio {
    struct msg {
        int flag;
        char id[24];
        unsigned int steps;
    };

    static map<string,unsigned int> status;

    struct session : public std::enable_shared_from_this<session> {
        tcp::socket fd;
        msg req,res;
        function<void (system::error_code ec, size_t len)> send,receive;

        session(tcp::socket sock) : fd(move(sock)) {}

        void start() {
            auto self(shared_from_this());
            send = [=](system::error_code ec, size_t len) {};
            receive = [=](system::error_code ec, size_t len) {
                if(ec) return;

                switch(req.flag) {
                case 0:{
                    cout << "run:" << endl;
                    req.steps++;
                    status[req.id] = req.steps;
                    async_write(fd,buffer(&req,sizeof(req)),send);
                    break;}
                default:{
                    auto runner_num = status.size();
                    cout << "peek:" << runner_num << endl;

                    async_write(fd,buffer(&runner_num,4),send);
                    for(auto& kv : status) {
                        fill_n(res.id,24,0);
                        kv.first.copy(res.id,kv.first.size(),0);
                        res.steps = kv.second;
                        async_write(fd,buffer(&res,sizeof(res)),send);
                        cout << res.id << ' ' << res.steps << endl;
                    }

                    async_read(fd,buffer(reinterpret_cast<char*>(&req),sizeof(req)),receive);
                    break;}
                }
            };

            async_read(fd,buffer(reinterpret_cast<char*>(&req),sizeof(req)),receive);
        }
    };

    static function<void(system::error_code ec,tcp::socket fd)> task;

    void server(unsigned short port) {
        io_service ios;
        tcp::acceptor server(ios,tcp::endpoint(tcp::v4(),port));

        task = [&](system::error_code ec,tcp::socket fd) {
            if(ec) return;
            cout << "new:" << fd.native_handle() << endl;
            make_shared<session>(move(fd))->start();
            server.async_accept(task);
        };

        server.async_accept(task);
        ios.run();
    }
}

#endif // SERVEROFASIO_H
