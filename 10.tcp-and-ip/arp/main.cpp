#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>

#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;

void arping(string dev,string destination) {
    auto fd = socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ARP));
    cout << "socket:" << strerror(errno) << endl;

    //允许广播
    int enable_broadcast = 1;
    setsockopt(fd,SOL_SOCKET,SO_BROADCAST,&enable_broadcast,sizeof(enable_broadcast));
    cout << "enable broadcast:" << strerror(errno) << endl;

    //获取网卡地址
    ifreq ifinfo;
    strncpy(ifinfo.ifr_name,dev.c_str(),dev.size());
    ioctl(fd,SIOCGIFHWADDR,&ifinfo); //socket io control get interface harderware address

    char ether[] {
        ifinfo.ifr_hwaddr.sa_data[0],
        ifinfo.ifr_hwaddr.sa_data[1],
        ifinfo.ifr_hwaddr.sa_data[2],
        ifinfo.ifr_hwaddr.sa_data[3],
        ifinfo.ifr_hwaddr.sa_data[4],
        ifinfo.ifr_hwaddr.sa_data[5]
    };

    stringstream source_mac;
    for(auto c : ether) source_mac << hex << setfill('0') << setw(2) << (0xff & c);
    cout << "source mac:" << source_mac.str() << endl;

    ioctl(fd,SIOCGIFADDR,&ifinfo);
    auto source_ip = reinterpret_cast<sockaddr_in*>(&ifinfo.ifr_addr)->sin_addr;
    cout << "source ip:" << inet_ntoa(source_ip) << endl;

    struct eth {
        char destination[6];//目标硬件地址，MAC地址, 广播地址:\xff\xff\xff\xff\xff\xff
        char source[6];//源硬件地址，MAC地址
        char type[2];//协议类型，ARP为0x0806
    };

    cout << "ether header size:" << dec << sizeof (eth) << endl;

    struct arp {
        eth header;
        char hardware_type[2];//硬件类型，0x0001为以太网
        char protocol_type[2];//协议类型，0x0800为IP协议
        char hardware_address_length[1];//硬件地址长度，即MAC地址的长度，6字节
        char protocol_address_length[1];//协议地址长度，即IP地址的长度，4字节
        char operation[2];//操作类型，请求为0x0001，响应为0x0002
        char source_mac[6];//源硬件地址，MAC地址
        char source_ip[4];//源逻辑地址，IP地址
        char destination_mac[6];//目标硬件地址，MAC地址
        char destination_ip[4];//目标逻辑地址，IP地址
    };

    cout << "arp header size:" << dec << sizeof (arp) << endl;

    arp req;
    memcpy(req.header.destination,"\xff\xff\xff\xff\xff\xff",6);
    memcpy(req.header.source,ether,6);
    memcpy(req.header.type,"\x08\x06",2);

    memcpy(req.hardware_type,"\x00\x01",2);
    memcpy(req.protocol_type,"\x08\x00",2);
    memcpy(req.hardware_address_length,"\x06",1);
    memcpy(req.protocol_address_length,"\x04",1);
    memcpy(req.operation,"\x00\x01",2);
    memcpy(req.source_mac,ether,6);
    memcpy(req.source_ip,&source_ip,4);
    memcpy(req.destination_mac,"\xff\xff\xff\xff\xff\xff",6);
    inet_aton(destination.c_str(),reinterpret_cast<in_addr*>(&req.destination_ip)); //获取IP地址

    cout << "if index:" << ifinfo.ifr_ifindex << endl;
    sockaddr_ll addr_ll;
    bzero(&addr_ll, sizeof(sockaddr_ll));
    addr_ll.sll_ifindex = ifinfo.ifr_ifindex; //网卡索引 ip addr
    addr_ll.sll_family = PF_PACKET; //链路层报文协议
    bind(fd,reinterpret_cast<sockaddr*>(&addr_ll),sizeof(addr_ll));
    cout << "bind:" << strerror(errno) << endl;

    cout << "arp size:" << dec << sizeof(req) << endl;
    cout << "send num:" << send(fd,&req,sizeof(req),0) << endl;
    cout << "send:" << strerror(errno) << endl;

    struct arp_res {
        ether_header e;
        ether_arp a;
    } res;

    do {
        cout << "recv num:" << recv(fd,&res,sizeof(res),0) << endl;
        cout << "recv:" << strerror(errno) << endl;

        stringstream eth_destination_mac,arp_destination_mac;
        for(auto c : res.e.ether_dhost) eth_destination_mac << hex << setfill('0') << setw(2) << (0xff & c);
        if(source_mac.str() != eth_destination_mac.str()) continue;

        for(auto c : res.a.arp_sha) arp_destination_mac << hex << setfill('0') << setw(2) << (0xff & c);
        cout << "destination mac:" << arp_destination_mac.str() << endl;
    } while('q' != getchar());

    close(fd);
    return;
}

int main()
{
    arping("enp3s0","192.168.1.197");
    return 0;
}
