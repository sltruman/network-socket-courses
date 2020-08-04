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
    },
    {
        kind: 'video',
        mimeType: 'video/VP9',
        clockRate: 90000,
        parameters:
        {
            'profile-id': 2,
            'x-google-start-bitrate': 1000
        }
    },
    {
        kind: 'video',
        mimeType: 'video/h264',
        clockRate: 90000,
        parameters:
        {
            'packetization-mode': 1,
            'profile-level-id': '4d0032',
            'level-asymmetry-allowed': 1,
            'x-google-start-bitrate': 1000
        }
    },
    {
        kind: 'video',
        mimeType: 'video/h264',
        clockRate: 90000,
        parameters:
        {
            'packetization-mode': 1,
            'profile-level-id': '42e01f',
            'level-asymmetry-allowed': 1,
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
    sock.on('authorize', (req, res) => {
        sock.userId = req.userId
        console.log('new:', sock.userId)
        res({ val: true, err: null })
    })

    sock.on('disconnect', () => {
        console.log('disconnect:', sock.userId ? sock.userId : sock.id)
    })

    sock.on('rtpCapabilities', async (req, res) => {
        res({ val: router.rtpCapabilities, err: null })
    })

    sock.on('newTransport', async (req, res) => {
        var transport = await router.createWebRtcTransport({
            listenIps: [{ ip: '172.21.157.67', announcedIp: '47.88.154.176' }],
            initialAvailableOutgoingBitrate: 1000000,
            minimumAvailableOutgoingBitrate: 600000,
            maxSctpMessageSize: 262144,
            maxIncomingBitrate: 1500000
        })

        var user = {}
        user[transport.id] = transport
        users[sock.userId] = user

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
        var user = users[sock.userId]
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
        var user = users[sock.userId]
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

        producer.observer.on('close', async () => {
            console.log('on producer close:', sock.userId)
        })

        res({ val: producer.id, err: null })
    })

    sock.on('consume', async (req, res) => {
        let user = users[sock.userId]
        if (user == undefined) {
            res({ val: null, err: 'user was not found!' })
            return
        }

        let transport = user[req.transportId]

        if (transport == undefined) {
            res({ val: null, err: 'transport was not found!' })
            return
        }

        let consumer = await transport.consume({ producerId: req.producerId, rtpCapabilities: req.rtpCapabilities })

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