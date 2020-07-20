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
    // 'room-id-1': {
    //     'user-id-1': {
    //         userId: 'user-id-1', roomId: 'room-id-1',
    //         nickname: '张三', server: { address: 'dungbeetles.xyz:8001' }, 
    //         producer: { video:null,videoPause:true } 
    //     }
    // }
}

var newRoomId = 0, serverIndex = 0;

io.on('connection', async sock => {
    console.log('new:', sock.id)

    sock.on('disconnect', () => {
        console.log('disconnect:', sock.id)
        quitRoom(sock)
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
        var user = { userId: sock.id, roomId: req.roomId, nickname: req.nickname, producer: {}, server: servers[serverIndex] }

        sock.roomId = req.roomId
        room[sock.id] = user

        res({ val: user, err: null })

        sock.emit('room', room)
        sock.join(req.roomId, (err) => {
            if (err) {
                console.error(err)
            }

            io.to(req.roomId).emit('join', user)
        })
    })

    sock.on('putProducerId', async (req, res) => {
        let room = rooms[sock.roomId]

        if (room == undefined) {
            res({ val: null, err: 'room was not found!' })
            return
        }

        let user = room[sock.id]
        if (user == undefined) {
            res({ val: null, err: 'user was not found!' })
            return
        }

        user.producer[req.kind] = req.producerId
        io.to(sock.roomId).emit('produce', user)
        res({ val: true, err: null })
    })

    sock.on('pause', async (req, res) => {
        let room = rooms[sock.roomId]

        if (room == undefined) {
            res({ val: null, err: 'room was not found!' })
            return
        }

        let user = room[sock.id]
        if (user == undefined) {
            res({ val: null, err: 'user was not found!' })
            return
        }

        res({ val: true, err: null })
        io.to(sock.roomId).emit('pause', { userId: user.userId, kind: req.kind })
    })


    sock.on('resume', async (req, res) => {
        let room = rooms[sock.roomId]
        if (room == undefined) {
            res({ val: null, err: 'room was not found!' })
            return
        }

        let user = room[sock.id]
        if (user == undefined) {
            res({ val: null, err: 'user was not found!' })
            return
        }

        res({ val: true, err: null })
        io.to(req.roomId).emit('resume', user)
    })

    function quitRoom(sock) {
        let room = rooms[sock.roomId]
        if (room == undefined) {
            return
        }

        sock.leave(sock.roomId)
        io.to(sock.roomId).emit('leave', room[sock.id])
        delete room[sock.id]
    }
})

web.listen(8000)