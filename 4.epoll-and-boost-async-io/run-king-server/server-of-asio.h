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
            send = [this,self](system::error_code ec, size_t len) {
                fd.close();
            };

            receive = [this,self](system::error_code ec, size_t len) {
                if(ec) {
                    cout << "close:" << fd.native_handle() << ' ' << ec.message() << endl;
                    fd.close();
                    return;
                }

                switch(req.flag) {
                case 0:{
                    cout << "run:" << req.steps << endl;
                    req.steps++;
                    status[req.id] = req.steps;
                    async_write(fd,buffer(&req,sizeof(req)),send);
                    break;}
                default:{
                    auto runner_num = status.size();
                    cout << "peek:" << runner_num << endl;

                    async_write(fd,buffer(&runner_num,4),[this,self](system::error_code ec, size_t len) {
                        if(ec) {
                            cout << "close:" << fd.native_handle() << ' ' << ec.message() << endl;
                            fd.close();
                            return;
                        }
                    });
                    for(auto& kv : status) {
                        fill_n(res.id,24,0);
                        kv.first.copy(res.id,kv.first.size(),0);
                        res.steps = kv.second;
                        async_write(fd,buffer(&res,sizeof(res)),[this,self](system::error_code ec, size_t len) {
                            if(ec) {
                                cout << "close:" << fd.native_handle() << ' ' << ec.message() << endl;
                                fd.close();
                                return;
                            }
                        });

                        cout << res.id << ' ' << res.steps << endl;
                    }
                    async_read(fd,buffer(reinterpret_cast<char*>(&req),sizeof(req)),receive);
                    break;}
                }
            };

            async_read(fd,buffer(reinterpret_cast<char*>(&req),sizeof(req)),receive);
        }
    };

    static function<void(system::error_code ec)> task;

    void server(unsigned short port) {
        io_service ios;
        tcp::socket fd_new(ios);
        tcp::acceptor server(ios,tcp::endpoint(tcp::v4(),port));

        task = [&](system::error_code ec) {
            if(ec) {
                cout << "accept:" << ec.message() << endl;
                return;
            }
            cout << "new:" << fd_new.native_handle() << endl;
            make_shared<session>(move(fd_new))->start();
            server.async_accept(fd_new,task);
        };

        server.async_accept(fd_new,task);
        ios.run();
    }
}

#endif // SERVEROFASIO_H
