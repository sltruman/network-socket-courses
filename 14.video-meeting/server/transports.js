const mediasoup = require('mediasoup')
const express = require('express')()

const codecs = [
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
]

var router

var transports = {
    'a': {
        '0':null,
        '1':null
    }
}

async function main() {
    var worker = await mediasoup.createWorker()
    router = await worker.createRouter({ codecs })
}

main()

express.get('/rtpCapabilities', async (req, res) => {
    res.send({ val: router.rtpCapabilities, err: null })
})

express.get('/newTransport', async (req, res) => {
    var transport = await router.createWebRtcTransport({
        listenIps: [{ ip: '127.0.0.1', announcedIp: '' }]
    })

    transports[req.sessionID][transport.id] = transport

    var ret = {
        val: {
            id: transport.id,
            iceParameters: transport.iceParameters,
            iceCandidates: transport.iceCandidates,
            dtlsParameters: transport.dtlsParameters
        },
        err: null
    }

    res.send(ret)
})

express.get('/connectTransport', async (req, res) => {
    var transport = transports[req.sessionID][req.query.transportId]

    if (transport == undefined) {
        res.send({ val: null, err: 'transport was not found!' })
        return;
    }

    await transport.connect({ dtlsParameters: req.query.dtlsParameters });
    res.send({ val: true, err: null })
})

express.get('/produce', async (req, res) => {
    var transport = transports[req.sessionID][req.query.transportId]

    if (transport == undefined) {
        res.send({ val: null, err: 'transport was not found!' })
        return;
    }

    var producer = await transport.produce(req.query.kind, req.query.rtpCapabilities);
    res.send({ val: producer.Id, err: null })
})

express.get('/consume', async (req, res) => {
    var transport = transports[req.sessionID][req.query.transportId]

    if (transport == undefined) {
        res.send({ val: null, err: 'transport was not found!' })
        return;
    }

    var consumer = await transport.consume({ producerId: req.query.producerId, rtpCapabilities: req.query.rtpCapabilities })

    var ret = {
        val: {
            id: consumer.id,
            kind: consumer.kind,
            rtpParameters: consumer.rtpParameters
        },
        err: null
    }

    res.send(ret)
})


express.listen(8001)
console.log('http://localhost:8001/')