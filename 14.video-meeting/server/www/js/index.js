var axios = require('axios').default

//创建会议
window.createMeeting = async function () {
    const res = await axios.get('http://localhost:8000/newRoom')
    var ret = res.data
    if (ret.err) {
        console.error(ret.err)
        return
    }

    var roomId = ret.val
    console.log('房间号：' + roomId)
    
    res = await axios.get('http://localhost:8000/joinRoom', {
        params: { roomId: roomId, nickname: '张三' }
    })

    if (ret.err) {
        console.error(ret.err)
        return
    }

    var server = res.data.val
    console.log('服务端：' + server)
}

//加入会议
window.joinMeeting = async function () {
    alert('')
}