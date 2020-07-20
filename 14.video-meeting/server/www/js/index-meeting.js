const mediasoup = require('mediasoup-client')
const io = require('socket.io-client')

module.exports = new function () {
    this.sock = null
    this.device = new mediasoup.Device()

    this.join = async (server) => {
        this.sock = io(`https://${server}`, {
            reconnection: true,             // whether to reconnect automatically
            reconnectionAttempts: Infinity, // number of reconnection attempts before giving up
            reconnectionDelay: 1000,        // how long to initially wait before attempting a new reconnection
            reconnectionDelayMax: 5000,     // maximum amount of time to wait between reconnection attempts. Each attempt increases the reconnection delay by 2x along with a randomization factor
            randomizationFactor: 0.5
        })

        this.sock.request = require('./lib/socket.io-promise').promise(this.sock)

        return new Promise(async (resolve) => {
            this.sock.on('connect', async () => {
                let res = await this.sock.request('rtpCapabilities')
                if (res.err) {
                    resolve({ err: 'failed to get rtp capabilities of server!', val: null })
                    return
                }

                await this.device.load({ routerRtpCapabilities: res.val })
                resolve({ err: null, val: true })
            })
        })
    }

    this.quit = async () => {
        if (this.sendTransport) sendTransport.close()
        if (this.recvTransport) recvTransport.close()
        this.sock.close()
    }

    this.sources = {
        'producerId': null
    }

    this.sendTransport = null
    this.produce = async (track) => {
        if (!this.sendTransport) {
            let res = await this.sock.request(`newTransport`)
            if (res.err) {
                return { val: null, err: 'failed to create transport at server!' }
            }

            this.sendTransport = this.device.createSendTransport(res.val)
            this.sendTransport.on('connect', async ({ dtlsParameters }, callback, errback) => {
                let res = await this.sock.request(`connectTransport`, {
                    transportId: this.sendTransport.id,
                    dtlsParameters: dtlsParameters
                })

                if (res.err) errbvack(res.err)
                else callback()
            })

            this.sendTransport.on('produce', async ({ kind, rtpParameters }, callback, errback) => {
                let res = await this.sock.request(`produce`, {
                    transportId: this.sendTransport.id,
                    kind: kind,
                    rtpParameters: rtpParameters
                })

                if (res.err) {
                    errback(ret.err)
                }

                let producerId = res.val
                callback({ id: producerId });
            })

            this.sendTransport.on('connectionstatechange', async (state) => {
                switch (state) {
                    case 'connecting':
                        console.log('sendTransport connecting')
                        break
                    case 'connected':
                        console.log('sendTransport connected')
                        break
                    case 'failed':
                        console.log('sendTransport failed')
                        this.sendTransport.close();
                        break
                    default: break;
                }
            })
        }

        let params = { track }
        let producer = await this.sendTransport.produce(params)
        this.sources[producer.id] = producer
        return { val: producer.id, err: null }
    }

    this.recvTransport = null
    this.consume = async (producerId) => {
        if (!this.recvTransport) {
            let res = await this.sock.request(`newTransport`)

            if (res.err) {
                return { val: null, err: 'failed to create transport at server!' }
            }

            this.recvTransport = this.device.createRecvTransport(res.val)
            this.recvTransport.on('connect', async ({ dtlsParameters }, callback, errback) => {
                let res = await this.sock.request('connectTransport', {
                    transportId: this.recvTransport.id,
                    dtlsParameters: dtlsParameters
                })

                if (res.err) errback(res.err)
                else callback()
            })

            this.recvTransport.on('connectionstatechange', async (state) => {
                switch (state) {
                    case 'connecting':
                        console.log('recvTransport connecting')
                        break
                    case 'connected':
                        console.log('recvTransport connected')
                        break
                    case 'failed':
                        console.log('recvTransport failed')
                        this.recvTransport.close();
                        break;

                    default: break;
                }
            })
        }

        if (this.sources[producerId]) {
            return { val: this.sources[producerId].track, err: null }
        }

        let res = await this.sock.request(`consume`, {
            producerId: producerId,
            rtpCapabilities: this.device.rtpCapabilities,
            transportId: this.recvTransport.id
        })

        if (res.err) {
            return { val: null, err: res.err }
        }

        let consumer = await this.recvTransport.consume({
            id: res.val.id,
            producerId: producerId,
            kind: res.val.kind,
            rtpParameters: res.val.rtpParameters
        })

        this.sources[producerId] = consumer
        return { val: consumer.track, err: null }
    }

    this.pause = async (producerId) => {
        this.sources[producerId].pause()
    }

    this.resume = async (producerId) => {
        this.sources[producerId].resume()
    }

    this.close = async (producerId) => {
        this.sources[producerId].close()
    }
}