const mediasoup = require('mediasoup')
const fs = require('fs')

const web = require('https').createServer({
    key: fs.readFileSync('./certs/ca.key'),
    cert: fs.readFileSync('./certs/ca.crt')
})

const io = require('socket.io')(web)

const mediaCodecs = [
    {
        kind: 'audio',
        mimeType: 'audio/opus',
        clockRate: 48000,
        channels: 2
    },
    {
        kind: 'video',
        mimeType: 'video/VP8',
        clockRate: 90000,
        parameters:
        {
            'x-google-start-bitrate': 1000
        }
    }
]

var users = {
    // 'user-id-1': {
    //     'transport-id-1': null,
    //     'transport-id-2': null
    // }
}

let router = null
async function main() {
    var worker = await mediasoup.createWorker()
    router = await worker.createRouter({ mediaCodecs })
}

main()

io.on('connection', async sock => {
    console.log('new:', sock.id)
    sock.on('disconnect', () => {
        console.log('disconnect:', sock.id)
    })

    sock.on('rtpCapabilities', async (req, res) => {
        res({ val: router.rtpCapabilities, err: null })
    })

    sock.on('newTransport', async (req, res) => {
        var transport = await router.createWebRtcTransport({
            listenIps: [{ ip: '172.21.157.67', announcedIp: '47.88.154.176' }]
        })

        var user = {}
        user[transport.id] = transport
        users[sock.id] = user

        res({
            val: {
                id: transport.id,
                iceParameters: transport.iceParameters,
                iceCandidates: transport.iceCandidates,
                dtlsParameters: transport.dtlsParameters
            },
            err: null
        })
    })

    sock.on('connectTransport', async (req, res) => {
        var user = users[sock.id]
        if (user == undefined) {
            res({ val: null, err: 'user was not found!' })
            return
        }

        var transport = user[req.transportId]
        if (transport == undefined) {
            res({ val: null, err: 'transport was not found!' })
            return;
        }

        await transport.connect({ dtlsParameters: req.dtlsParameters });
        res({ val: true, err: null })
    })

    sock.on('produce', async (req, res) => {
        var user = users[sock.id]
        if (user == undefined) {
            res({ val: null, err: 'user was not found!' })
            return
        }

        var transport = user[req.transportId]
        if (transport == undefined) {
            res({ val: null, err: 'transport was not found!' })
            return
        }

        var producer = await transport.produce({ kind: req.kind, rtpParameters: req.rtpParameters })
        res({ val: producer.id, err: null })
    })

    sock.on('consume', async (req, res) => {
        var transport = users[sock.id][req.transportId]

        if (transport == undefined) {
            res({ val: null, err: 'transport was not found!' })
            return
        }

        var consumer = await transport.consume({ producerId: req.producerId, rtpCapabilities: req.rtpCapabilities })

        res({ 
            val: {
                id: consumer.id,
                kind: consumer.kind,
                rtpParameters: consumer.rtpParameters
            },
            err: null
        })
    })
})

web.listen(8001)