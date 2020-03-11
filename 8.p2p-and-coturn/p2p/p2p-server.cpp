#include <iostream>
#include <map>
#include <boost/asio.hpp>

using namespace std;

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;

int main(int argc,char* argv[])
{
    if(argc < 2) return 1;
    auto port = stoi(argv[1]);

    io_service ios;
    udp::endpoint self(udp::v4(),port),other;
    udp::socket server(ios,self);

    struct {
        unsigned short type;//0:req 1:text
        unsigned short id;
        unsigned short peer_id;
        unsigned char out_peer_address[16];
        unsigned short out_peer_port;
        char out_peer_text[32];
    }msg;

    map<unsigned short,pair<string,unsigned short>> records;
    while(true) {
        server.receive_from(buffer(&msg,sizeof(msg)),other);
        records[msg.id] = make_pair(other.address().to_v4().to_string(),other.port());
        cout << "client " << msg.id << ' ' << other.address().to_v4().to_string() << ':' << other.port() << endl;
        switch(msg.type){
        case 0:{
            auto it = records.find(msg.peer_id);
            if(records.end() != it) {
                it->second.first.copy((char*)msg.out_peer_address,it->second.first.size());
                msg.out_peer_port = it->second.second;
            }

            server.send_to(buffer(&msg,sizeof (msg)),other);
            break;}
        case 1:
            break;
        }
    }

    return 0;
}

