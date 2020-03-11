var express = require('express')
var app = express()
var http = require('http').createServer(app)
var signaling = require('./signaling-server')(http)

app.use(express.static('www'))

http.listen(8000, () => {
    console.log('http://dungbeetles.xyz:8000/')
})