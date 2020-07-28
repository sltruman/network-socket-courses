const mediasoup = require('mediasoup-client')
const io = require('socket.io-client')
let device = new mediasoup.Device()
let ioTransports = null
let sendTransport = null
let recvTransport = null

this.sources = {
    'producerId': null
}

module.exports = new function () {
    this.connected = false
    this.connect = async (server, userId) => {
        this.connected = true
        ioTransports= io(`https://${server}`)
        ioTransports.request = require('./lib/socket.io-promise').promise(ioTransports)
        ioTransports.on('connect', async () => {
            if (!device.loaded) {         //初始化设备
                let res = await ioTransports.request('rtpCapabilities')
                if (res.err) {
                    return
                }

                await device.load({ routerRtpCapabilities: res.val })
            }

            ioTransports.userId = userId
            res = await ioTransports.request(`authorize`, { userId: userId })//用户认证

            if (res.err) {
                console.err(res.err)
                sock.disconnect()
                window.location.href = 'index.html' //返回准备界面
            }

            console.log('reconnected userId:' + ioTransports.userId)
        })

        ioTransports.on('disconnect', async () => { })
    }

    this.quit = async () => {
        if (this.sendTransport) this.sendTransport.close()
        if (this.recvTransport) this.recvTransport.close()
        this.ioTransports.close()
    }

    this.produce = async (track) => {
        if (!this.sendTransport) {
            let res = await ioTransports.request(`newTransport`)
            if (res.err) {
                return { val: null, err: 'failed to create transport at server!' }
            }

            let sendTransport = device.createSendTransport(res.val)
            sendTransport.on('connect', async ({ dtlsParameters }, callback, errback) => {
                let res = await ioTransports.request(`connectTransport`, {
                    transportId: sendTransport.id,
                    dtlsParameters: dtlsParameters
                })

                if (res.err) errbvack(res.err)
                else callback()
            })

            sendTransport.on('produce', async ({ kind, rtpParameters }, callback, errback) => {
                let res = await ioTransports.request(`produce`, {
                    transportId: sendTransport.id,
                    kind: kind,
                    rtpParameters: rtpParameters
                })

                if (res.err) {
                    errback(ret.err)
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
                        sendTransport.close();
                        break
                    default: break;
                }
            })
        }

        let params = { track }
        let producer = await sendTransport.produce(params)
        sources[producer.id] = producer
        return { val: producer.id, err: null }
    }


    this.consume = async (producerId) => {
        if (!recvTransport) {
            let res = await ioTransports.request(`newTransport`)

            if (res.err) {
                return { val: null, err: 'failed to create transport at server!' }
            }

            recvTransport = device.createRecvTransport(res.val)
            recvTransport.on('connect', async ({ dtlsParameters }, callback, errback) => {
                let res = await ioTransports.request('connectTransport', {
                    transportId: recvTransport.id,
                    dtlsParameters: dtlsParameters
                })

                if (res.err) errback(res.err)
                else callback()
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
                        break;

                    default: break;
                }
            })
        }

        if (sources[producerId]) {
            return { val: sources[producerId].track, err: null }
        }

        let res = await sock.request(`consume`, {
            producerId: producerId,
            rtpCapabilities: device.rtpCapabilities,
            transportId: recvTransport.id
        })

        if (res.err) {
            return { val: null, err: res.err }
        }

        let consumer = await recvTransport.consume({
            id: res.val.id,
            producerId: producerId,
            kind: res.val.kind,
            rtpParameters: res.val.rtpParameters
        })

        sources[producerId] = consumer
        return { val: consumer.track, err: null }
    }

    this.close = async (producerId) => {
        sources[producerId].close()
    }
}
