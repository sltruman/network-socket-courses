window.onload = () => {
    var servers = {
        "iceServers": [{
            "urls": ["stun:dungbeetles.xyz"]
        }, {
            "urls": ["turn:dungbeetles.xyz"],
            "username": "truman",
            "credential": "xf.sky.l"
        }]
    }

    var socket = null
    var connection = null
    var channel = null
    var localId = null

    function initChannelCallback(channel) {
        console.log('initChannelCallback')

        channel.onopen = async () => {
            console.log('open')
            document.querySelector('.status').innerHTML = '通道已建立，开始通讯！'
            document.querySelector('.connect').disabled = true
            document.querySelector('.close').disabled = false
            var alive = () => {
                console.log(channel.readyState)

                if (channel.readyState == 'open')
                    setTimeout(alive, 1000)
                else
                    document.querySelector('.close').click()
            }

            setTimeout(alive, 1000)
        }

        channel.onmessage = async (e) => {
            console.log('msg', e.data)
            var msgList = document.querySelector('.msgList')
            msgList.value += '\n' + e.data
        }

        channel.onclose = async () => {
            console.log('close')
            document.querySelector('.status').innerHTML = '通道已关闭，停止通讯！'
            document.querySelector('.connect').disabled = false
            document.querySelector('.close').disabled = true
        }
    }

    var student = async () => {
        if (socket) socket.close()
        socket = io('http://localhost:8000')

        if (connection) connection.close()

        connection = new RTCPeerConnection(servers)
        console.log('student')

        socket.on('takeId', async (id) => {
            console.log(`takeId ${id}`)
            localId = id
            document.querySelector('.localId').innerHTML = localId
        })

        var studentCandidate = null
        connection.onicecandidate = async (e) => {
            if (e.candidate == null) {
                socket.emit('register', offer, studentCandidate)
                console.log('studentCandidate', offer, studentCandidate)
            }
            else {
                studentCandidate = e.candidate
            }
        }

        if (channel) channel.close()
        channel = connection.createDataChannel('sendDataChannel')
        console.log('Created send data channel')

        var offer = await connection.createOffer()
        await connection.setLocalDescription(offer)

        socket.on('shareScreen', async (anser, teacherCandidate) => {
            initChannelCallback(channel)
            console.log(`teacherCandidate`, anser, teacherCandidate)
            await connection.setRemoteDescription(anser)
            await connection.addIceCandidate(teacherCandidate)
        })
    }

    var teacher = async () => {
        if (socket) socket.close()
        socket = io('http://localhost:8000')
        if (connection) connection.close()
        connection = new RTCPeerConnection(servers)
        console.log('teacher')

        if (channel) channel.close()
        channel = connection.createDataChannel('sendDataChannel')
        console.log('Created send data channel')

        var teacherCandidate = null
        var anser = null
        socket.on('takeOffer', async (offer, studentCandidate) => {
            console.log('takeOffer', offer, studentCandidate)
            connection.onicecandidate = async (e) => {
                if (e.candidate == null) {
                    console.log('teacherCandidate', teacherCandidate)
                    var studentId = document.querySelector('.remoteID').value
                    await connection.addIceCandidate(studentCandidate)
                    socket.emit('getScreen', studentId, anser, teacherCandidate)
                }
                else {
                    teacherCandidate = e.candidate
                }
            }

            await connection.setRemoteDescription(offer)
            console.log('setRemoteDesc')
            anser = await connection.createAnswer()
            console.log('createAnser')
            await connection.setLocalDescription(anser)
        })

        socket.on('wrongTakeOffer', async () => {
            console.log('takeOffer')
            document.querySelector('.status').innerHTML = '无效的ID！'
        })

        connection.ondatachannel = (e) => {
            console.log('Created receive data channel')
            channel = e.channel
            initChannelCallback(e.channel)
        }
    }

    student()

    document.querySelector('.connect').addEventListener('click', async () => {
        console.log('connect')

        await teacher()

        var studentId = document.querySelector('.remoteId').value
        socket.emit('requestScreen', studentId)
    })

    document.querySelector('.send').addEventListener('click', () => {
        if (channel.readyState != 'open') document.querySelector('.close').click()
        var msg = document.querySelector('.msg').value
        console.log('send', msg)
        channel.send(localId + '：' + msg)
        document.querySelector('.msgList').value += '\n本机：' + msg
    })

    document.querySelector('.close').addEventListener('click', () => {
        console.log('close')
        channel.close()
        connection.close()
        socket.close()
        student()
    })
}