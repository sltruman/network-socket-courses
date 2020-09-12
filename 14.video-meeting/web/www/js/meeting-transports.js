const mediasoup = require('mediasoup-client')
const io = require('socket.io-client')
let device = new mediasoup.Device()
let ioTransports = null
let sendTransport = null
let recvTransport = null

let mediaStreams = {
    'track.id': null
}

async function onConnectedEvent() {
    console.log('connected to transport services!')
    if (device.loaded)  //初始化设备
        return

    let res = await ioTransports.request('rtpCapabilities')
    if (res.err) {
        console.error(res.err)
        return
    }

    await device.load({ routerRtpCapabilities: res.val })

    res = await ioTransports.request(`newTransport`)
    if (res.err) {
        console.error(res.err)
        return;
    }

    sendTransport = device.createSendTransport(res.val)
    sendTransport.on('connect', async ({ dtlsParameters }, callback, errback) => {
        let res = await ioTransports.request(`connectTransport`, {
            transportId: sendTransport.id,
            dtlsParameters: dtlsParameters
        })

        if (res.err) {
            console.error(res.err)
            errbvack(res.err)
            return
        }

        callback()
    })

    sendTransport.on('produce', async ({ kind, rtpParameters }, callback, errback) => {
        let res = await ioTransports.request(`produce`, {
            transportId: sendTransport.id,
            kind: kind,
            rtpParameters: rtpParameters
        })

        if (res.err) {
            console.error(res.err)
            errback(res.err)
            return
        }

        let producerId = res.val
        callback({ id: producerId });
    })

    sendTransport.on('connectionstatechange', async (state) => {
        switch (state) {
            case 'connecting':
                console.log('sendTransport connecting')
                break
            case 'connected':
                console.log('sendTransport connected')
                break
            case 'failed':
                console.log('sendTransport failed')
                sendTransport.close()
                sendTransport = null
                break
            default: break;
        }
    })

    res = await ioTransports.request(`newTransport`)
    if (res.err) {
        console.error(res.err)
        return;
    }

    recvTransport = device.createRecvTransport(res.val)
    recvTransport.on('connect', async ({ dtlsParameters }, callback, errback) => {
        let res = await ioTransports.request('connectTransport', {
            transportId: recvTransport.id,
            dtlsParameters: dtlsParameters
        })

        if (res.err) {
            console.error(res.err)
            errback(res.err)
            return
        }

        callback()
    })

    recvTransport.on('connectionstatechange', async (state) => {
        switch (state) {
            case 'connecting':
                console.log('recvTransport connecting')
                break
            case 'connected':
                console.log('recvTransport connected')
                break
            case 'failed':
                console.log('recvTransport failed')
                recvTransport.close()
                recvTransport = null
                break;

            default: break;
        }
    })
}

async function onDisconnectedEvent() {
    console.log('disconnected from transport services!')

    // for (id in mediaStreams) {
    //     await mediaStreams[id].transmitting.close()
    //     delete mediaStreams[id]
    // }
}

module.exports = new function () {
    this.connect = (server) => {
        ioTransports = io(`https://${server}`)
        ioTransports.request = require('./lib/socket.io-promise').promise(ioTransports)
        ioTransports.on('connect', onConnectedEvent)
        ioTransports.on('disconnect', onDisconnectedEvent)
    }

    this.produce = (track, callback) => {
        if (!sendTransport) {
            setTimeout(this.produce, 1000, track, callback)
            return
        }

        sendTransport.produce({ track }).then((producer) => {
            track.transmitting = producer
            mediaStreams[track.id] = track
            callback({ val: producer.id, err: null })
        }).catch(err => {
            callback({ val: null, err: err })
        })
    }

    this.consume = (producerId, callback) => {
        if (!recvTransport) {
            setTimeout(this.consume, 1000, producerId, callback)
            return
        }

        ioTransports.emit(`consume`, {
            producerId: producerId,
            rtpCapabilities: device.rtpCapabilities,
            transportId: recvTransport.id
        }, ({ val, err }) => {
            recvTransport.consume({
                id: val.id,
                producerId: producerId,
                kind: val.kind,
                rtpParameters: val.rtpParameters
            }).then(consumer => {
                consumer.track.transmitting = consumer
                mediaStreams[consumer.track.id] = consumer.track
                callback({ val: consumer.track, err: null })
            }).catch(err => {
                callback({ val: null, err: err })
            })
        })
    }

    this.remove = (track) => {
        console.log(mediaStreams)
        mediaStreams[track.id].transmitting.close()
        delete mediaStreams[track.id].transmitting
        delete mediaStreams[track.id]
    }
}
