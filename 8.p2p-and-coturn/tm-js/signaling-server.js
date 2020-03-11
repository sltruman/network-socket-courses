module.exports = (server) => {
    var io = require('socket.io')(server)
    var clientIds = {}
    io.on('connect', (client) => {
        console.log(`${client.id} online`)
        client.on('disconnect', () => {
            console.log(`${client.id} offline`)
            delete clientIds[client.name]
        })

        client.on('register', (offer, candidate) => {
            if (candidate == null) return

            var name = Math.floor(Math.random() * 10000) + 10000
            console.log('register:', name)
            client.offer = offer
            client.candidate = candidate
            client.name = name
            clientIds[name] = client
            client.emit('takeId', name)
        })

        client.on('requestScreen', (studentName) => {
            console.log('requestScreen:', studentName)
            try {
                var studentClient = clientIds[studentName]
                client.emit('takeOffer', studentClient.offer, studentClient.candidate)
            } catch (e) {
                client.emit('wrongTakeOffer')
            }
        })

        client.on('getScreen', (studentName, anser, teacherCandidate) => {
            console.log('getScreen')
            var studentClient = clientIds[studentName]
            studentClient.emit('shareScreen', anser, teacherCandidate)
        })
    })
}