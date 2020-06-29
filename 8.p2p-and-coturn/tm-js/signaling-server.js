module.exports = (server) => {
    var io = require('socket.io')(server)
    var clientIds = {}  //map，储存着所有的客户端信息

    io.on('connect', function (client) { //如果有用户连接上来了，则执行回调函数

        console.log(`${client.id} online`)

        client.on('disconnect', () => { //如果客户端断开连接了，则删除客户端信息
            console.log(`${client.id} offline`)
            delete clientIds[client.name]
        })

        client.on('register', (offer, candidate) => { //如果客户端请求注册信息
            if (candidate == null) return //判断客户信息是否有效

            var name = Math.floor(Math.random() * 10000) + 10000 //随机生成客户标识

            console.log('register:', name)
            client.offer = offer
            client.candidate = candidate
            client.name = name   
            clientIds[name] = client  //储存到数据库
            client.emit('takeId', name) //返回用户标识给客户
        })

        client.on('requestScreen', (studentName) => { //如果客户请求拨号
            console.log('requestScreen:', studentName)
            try {
                var studentClient = clientIds[studentName] //根据用户标识从数据库获取详细信息
                client.emit('takeOffer', studentClient.offer, studentClient.candidate)
            } catch (e) {
                client.emit('wrongTakeOffer')
            }
        })

        client.on('getScreen', (studentName, anser, teacherCandidate) => {
            console.log('getScreen')
            var studentClient = clientIds[studentName] //根据要被通讯的学生名，得到学生信息
            studentClient.emit('shareScreen', anser, teacherCandidate)  //把老师的电话号码发送给学生
        })
    })
}