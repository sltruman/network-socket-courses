var express = require('express')   //引用express web框架
var app = express()
var http = require('http').createServer(app)  //从express实例得到http服务端的实例
var signaling = require('./signaling-server')(http) //设置信令服务接口

app.use(express.static('www')) //设置html静态页面

http.listen(8000, () => {  //监听8000端口
    console.log('http://dungbeetles.xyz:8000/')
})