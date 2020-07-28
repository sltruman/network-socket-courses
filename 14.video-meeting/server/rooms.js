const fs = require('fs')

const web = require('https').createServer({
    key: fs.readFileSync('./certs/ca.key'),
    cert: fs.readFileSync('./certs/ca.crt')
})

const io = require('socket.io')(web)

var servers = [
    { address: 'dungbeetles.xyz:8001' }
]

var rooms = {
    'room-id-1': {
        'user-id-1': {
            userId: 'user-id-1', roomId: 'room-id-1',
            nickname: '张三', server: { address: 'dungbeetles.xyz:8001' },
            producers: {
                video: { id: '', live: true },
                desktop: { id: '', live: true },
                audio: { id: '', live: true }
            }
        }
    }
}

var newRoomId = 0, serverIndex = 0;

io.on('connection', async sock => {
    sock.on('disconnect', () => {
        if (sock.userId == undefined) return
        console.log('disconnect:', sock.userId)
        let room = rooms[sock.roomId]
        if (room == undefined) return
        sock.leave(sock.roomId)
        io.to(sock.roomId).emit('leave', room[sock.userId])
        delete room[sock.userId]
    })

    sock.on('authorize', (req, res) => {
        sock.userId = req.userId
        console.log('new:', sock.userId)
        res({ val: true, err: null })
    })

    sock.on('newRoom', async (req, res) => {
        newRoomId++
        rooms[newRoomId] = {}
        res({ val: newRoomId, err: null })
    })

    sock.on('joinRoom', async (req, res) => {
        var room = rooms[req.roomId]

        if (room == undefined) {
            res({ val: null, err: 'room was not found!' })
            return
        }

        serverIndex = ++serverIndex == servers.length ? 0 : serverIndex
        var user = { userId: sock.userId, roomId: req.roomId, nickname: req.nickname, producer: {}, server: servers[serverIndex] }

        sock.roomId = req.roomId
        sock.join(req.roomId, err => {
            if (err) {
                console.err(err)
                res({ val: null, err: err })
            } else {
                res({ val: user, err: null })
                io.to(req.roomId).emit('join', user)
                for (userId in room) sock.emit('join', room[userId])
                room[sock.userId] = user
            }
        })
    })

    sock.on('produce', async (req, res) => {
        let room = rooms[sock.roomId]

        if (room == undefined) {
            res({ val: null, err: 'room was not found!' })
            return
        }

        let user = room[sock.userId]
        if (user == undefined) {
            res({ val: null, err: 'user was not found!' })
            return
        }

        user.producer[req.kind] = { id: req.producerId, live: req.live }
        res({ val: true, err: null })

        io.to(sock.roomId).emit('produce', user)
    })
})

web.listen(8000)