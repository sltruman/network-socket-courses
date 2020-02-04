#ifndef SERVEROFASIO_H
#define SERVEROFASIO_H

#include <iostream>
#include <map>
#include <memory>
using namespace std;

#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <zmq.h>

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
    static void *context,*sock;

    struct session : public std::enable_shared_from_this<session> {
        tcp::socket fd;
        msg req,res;
        function<void (system::error_code ec, size_t len)> send,receive;
        function<void()> reply;

        session(tcp::socket sock) : fd(std::move(sock)) {}

        void start() {
            auto self(shared_from_this());

            reply = [=] {
                string res(64,'\0');
                if(-1 == zmq_recv(sock,const_cast<char*>(res.data()),res.size(),ZMQ_DONTWAIT) && errno == EAGAIN)
                    fd.get_io_service().post(reply);
            };

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
                case 0:
                    cout << "run:" << req.steps << endl;
                    req.steps++;
                    status[req.id] = req.steps;
                    async_write(fd,buffer(&req,sizeof(req)),send);
                    fd.get_io_service().post([=]{
                        boost::format fmt(R"({ "type":"set", "id":"%s", "value":%d })");
                        fmt = fmt % req.id % req.steps;
                        if(0 < zmq_send(sock,fmt.str().data(),fmt.str().size(),0))
                            fd.get_io_service().post(reply);
                    });
                    break;
                }
            };

            async_read(fd,buffer(reinterpret_cast<char*>(&req),sizeof(req)),receive);
        }
    };

    static function<void(system::error_code ec)> task;

    void server(unsigned short port) {
        context = zmq_ctx_new();
        sock = zmq_socket(context,ZMQ_REQ);
        zmq_connect(sock,"tcp://127.0.0.1:27017");

        io_service ios;
        tcp::socket fd_new(ios);
        tcp::acceptor server(ios,tcp::endpoint(tcp::v4(),port));

        task = [&](system::error_code ec) {
            if(ec) {
                cout << "accept:" << ec.message() << endl;
                return;
            }
            cout << "new:" << fd_new.native_handle() << endl;
            make_shared<session>(std::move(fd_new))->start();
            server.async_accept(fd_new,task);
        };

        server.async_accept(fd_new,task);
        ios.run();
    }
}

#endif // SERVEROFASIO_H
