module.exports.listenPort = 8000
module.exports.servers = [
    {
        //传送服务地址,用于负载均衡
        address: '192.168.16.224:8001'
    }
]