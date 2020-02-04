const zmq = require('zeromq')
var datastore = {
}

var main = async () => {
    var sock = zmq.socket('rep')
    sock.bindSync('tcp://*:27017')

    sock.on('message', async (msg) => { 
        var req = JSON.parse(msg)
        console.log(req)
        switch (req.type) {
            case 'set':
                datastore[req.id] = req.value;
                sock.send(JSON.stringify({ error: null }))
                break;
            case 'get':
                sock.send(JSON.stringify({ ret: datastore, error: null }))
                break;
            default:
                sock.send(JSON.stringify({ error: '未知请求！' }))
                break;
        }
    })
}

main()