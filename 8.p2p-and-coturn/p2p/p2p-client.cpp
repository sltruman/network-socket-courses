#include <iostream>
#include <map>
#include <thread>
#include <boost/asio.hpp>

using namespace std;

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace boost;

int main(int argv,char* argc[])
{
    if(argv != 3) return 1;
    unsigned short id = stoi(argc[1]),peer_id = stoi(argc[2]);

    io_service ios;
    udp::endpoint self(udp::v4(),id);
    udp::socket client(ios,self);
    struct msg {
        unsigned short type;//0:req 1:text
        unsigned short id;
        unsigned short peer_id;
        unsigned char out_peer_address[16];
        unsigned short out_peer_port;
        char out_peer_text[32];
    };

    auto t = thread([&]() {
        while(true) {
            msg req_peer = {0,id,peer_id,"",0,""};
            client.send_to(buffer(&req_peer,sizeof(req_peer)),udp::endpoint(ip::address_v4::from_string("47.115.170.39"),8000));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    while(true) {
        udp::endpoint ep;
        msg res_peer;
        client.receive_from(buffer(&res_peer,sizeof(res_peer)),ep);
        switch(res_peer.type) {
        case 0:{
            if(string((char*)res_peer.out_peer_address).empty()) break;
            udp::endpoint peer(ip::address_v4::from_string((char*)res_peer.out_peer_address),res_peer.out_peer_port);

            msg tunnel {2,id,peer_id,"",0,""};
            for(int i=0;i < 3;i++) {
                client.send_to(buffer(&tunnel,sizeof(tunnel)), peer);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            msg send_text {1,id,peer_id,"",0,"this is peer's text!"};
            client.send_to(buffer(&send_text,sizeof(send_text)), peer);
            break;}
        case 1:
            cout << res_peer.out_peer_text << endl;
            break;
        default:
            cout << "tunnel!" << endl;
            break;
        }
    }


    return 0;
}
