const Express = require('express')
const https = require('https');
const session = require('express-session');
const cookie = require('cookie-parser');
const fs = require('fs');
const express = Express()

//同步读取密钥和签名证书
var options = {
    key: fs.readFileSync('./certs/ca.key'),
    cert: fs.readFileSync('./certs/ca.crt')
}

var web = https.createServer(options,express);

express.use(cookie())
express.use(session({
    secret: 'hello world!',
    name: 'video-meeting',   //这里的name值得是cookie的name，默认cookie的name是：connect.sid
    cookie: { maxAge: 80000 },  //设置maxAge是80000ms，即80s后session和相应的cookie失效过期
    resave: false,
    saveUninitialized: true
}))

var servers = [
    { address: 'dungbeetles.xyz:8001' }
]

var rooms = {
    'skyroom': {
        'zhangsan': { nickname: '张三', server: { address: 'dungbeetles.xyz:8001' }, producerId: 0, activeTime: new Date },
    }
}

var newRoomId = 0, serverIndex = 0;

express.get('/newRoom', async (req, res) => {
    res.setHeader('Access-Control-Allow-Credentials', true)
    res.setHeader('Access-Control-Allow-Origin', req.headers.origin ? req.headers.origin : '*')

    rooms[newRoomId] = {}
    res.send({ val: newRoomId, err: null })
    newRoomId++

    console.log(rooms)
})

express.get('/joinRoom', async (req, res) => {
    res.setHeader('Access-Control-Allow-Credentials', true)
    res.setHeader('Access-Control-Allow-Origin', req.headers.origin ? req.headers.origin : '*')

    var room = rooms[req.query.roomId]

    if (room == undefined) {
        res.send({ val: null, err: 'room was not found!' })
        return
    }

    serverIndex = ++serverIndex == servers.length ? 0 : serverIndex
    var userDetail = { nickname: req.query.nickname, server: servers[serverIndex], activeTime: new Date() }

    rooms[req.query.roomId][req.sessionID] = userDetail
    res.send({ val: userDetail, err: null })
    console.log(rooms[req.query.roomId])
})

express.get('/updateProducerId', async (req, res) => {
    res.setHeader('Access-Control-Allow-Credentials', true)
    res.setHeader('Access-Control-Allow-Origin', req.headers.origin ? req.headers.origin : '*')

    if (rooms[req.query.roomId] == undefined) {
        res.send({ val: null, err: 'room was not found!' })
        return
    }

    if (rooms[req.query.roomId][req.sessionID] == undefined) {
        res.send({ val: null, err: 'user was not found!' })
        return
    }

    rooms[req.query.roomId][req.sessionID].producerId = req.query.producerId
    res.send({ val: true, err: null })
})

express.get('/queryRoom', async (req, res) => {
    res.setHeader('Access-Control-Allow-Credentials', true)
    res.setHeader('Access-Control-Allow-Origin', req.headers.origin ? req.headers.origin : '*')

    if (rooms[req.query.roomId] == undefined) {
        res.send({ val: null, err: 'room was not found!' })
        return
    }

    if (rooms[req.query.roomId][req.sessionID] == undefined) {
        res.send({ val: null, err: 'user was not found!' })
        return
    }

    rooms[req.query.roomId][req.sessionID].activeTime = new Date
    res.send({ val: rooms[req.query.roomId], err: null })
})

express.get('/quitRoom', async (req, res) => {
    res.setHeader('Access-Control-Allow-Credentials', true)
    res.setHeader('Access-Control-Allow-Origin', req.headers.origin ? req.headers.origin : '*')

    if (rooms[req.query.roomId] == undefined) {
        res.send({ val: null, err: 'room was not found!' })
        return
    }

    if (rooms[req.query.roomId][req.sessionID] == undefined) {
        res.send({ val: null, err: 'user was not found!' })
        return
    }

    delete rooms[req.query.roomId][req.sessionID]
    res.send({ val: true, err: null })
    console.log(rooms[req.query.roomId])
})

//资源清理程序
setInterval(() => {
    var currentTime = new Date
    for (roomId in rooms) {
        var room = rooms[roomId]
        for (userId in room) {
            var user = room[userId]
            var lastTime = user.activeTime
            var offsetTime = new Date(currentTime - lastTime)

            if (offsetTime.getMinutes() > 0) { //超过一分钟
                console.log('退出房间：' + userId + ' ' + JSON.stringify(user))
                delete room[userId]
            }
        }
    }
}, 2000)

web.listen(8000)