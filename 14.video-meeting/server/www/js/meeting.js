const ioRooms = require('socket.io-client')('https://dungbeetles.xyz:8000')
ioRooms.request = require('./lib/socket.io-promise').promise(ioRooms)
const transports = require('./meeting-transports')
const Vue = require('vue')

let vueMessage = new Vue({

})

var vueApp = new Vue({
  el: '#app',
  data: {
    message: '',
    microphoneActivable: false,
    cameraActivable: false,
    screenActivable: false,
    users: {
      // 'user-id': { userId, nickname, producer: { video, audio, desktop }, muted, primary }
    }
  },
  methods: {
    microphoneSwitch: async () => {
      $('#microphoneSwitch').toggleClass('active')

      this.microphoneActivable = !this.microphoneActivable
      if (!this.microphoneActivable) {
        return
      }

      let stream = await navigator.mediaDevices.getUserMedia({ video: false, audio: true })
      let audioTrack = stream.getAudioTracks()[0]

      let res = await meeting.produce(audioTrack)

      if (res.err) {
        alert(res.err)
        this.microphoneActivable = !this.microphoneActivable
        return
      }

      res = await sock.request(`putProducerId`, {
        producerId: res.val,
        kind: 'audio'
      })

      if (res.err) {
        alert(res.err)
        this.microphoneActivable = !this.microphoneActivable
        return
      }
    },
    cameraSwitch: async () => {
      $('#cameraSwitch').toggleClass('active')

      this.cameraActivable = !this.cameraActivable
      console.log(this.cameraActivable)

      let user = vueApp.users[sock.userId]
      if (!this.cameraActivable) {      //暂停视频
        let res = await sock.request(`pause`, { kind: 'video' })

        if (res.err) {
          alert(res.err)
          this.cameraActivable = !this.cameraActivable
        }

        await meeting.pause(user.producer.video)
        return
      }

      let userStream = user.stream
      if (userStream.getVideoTracks().length) { //恢复视频
        let res = await sock.request(`resume`, { kind: 'video' })

        if (res.err) {
          alert(res.err)
          this.cameraActivable = !this.cameraActivable
        }

        await meeting.resume(user.producer.video)
        return
      }

      //推送视频
      let stream = await navigator.mediaDevices.getUserMedia({ video: true, audio: false })
      let videoTrack = stream.getVideoTracks()[0]
      userStream.addTrack(videoTrack)

      let res = await meeting.produce(videoTrack)

      if (res.err) {
        alert(res.err)
        this.cameraActivable = !this.cameraActivable
        return
      }

      res = await sock.request(`putProducerId`, {
        producerId: res.val,
        kind: 'video'
      })

      if (res.err) {
        alert(res.err)
        this.cameraActivable = !this.cameraActivable
      }

      if (user.primary)
        document.querySelector('#primary').srcObject = await user.stream
      else
        $(`#video-${req.userId}`).srcObject = await user.stream
    },
    screenSwitch: async () => {
      $('#screenSwitch').toggleClass('active')
      this.screenActivable = !this.screenActivable
    },
    sendMessage: async () => { },
    quitMeeting: async () => {
      if (!confirm("Are you sure to quit?"))
        return

      ioRooms.disconnect()
      ioTransports.disconnect()
      window.location.href = 'index.html'
    }
  }
})

ioRooms.on('connect', async () => {
  let roomId = window.localStorage.getItem('roomId')
  let userId = window.localStorage.getItem('userId')
  if (!userId || !roomId) window.location.href = 'index.html'

  ioRooms.userId = userId
  let res = await ioRooms.request(`authorize`, { userId: userId }) //用户认证

  if (res.err) {
    ioRooms.disconnect()
    alert(res.err)
    window.location.href = 'index.html' //返回准备界面
  }

  console.log('authorize:', res)
  console.log('reconnected userId:' + ioRooms.userId)

  let nickname = window.localStorage.getItem('nickname')
  res = await ioRooms.request('joinRoom', {
    roomId: roomId, nickname: nickname
  })

  if (res.err) {
    ioRooms.disconnect()
    alert(res.err)
    window.location.href = 'index.html'
  }

  console.log('joinRoom:', res)
  if (transports.connected) return
  await transports.connect(res.val.server.address, userId) //连接传送管理服务
})

ioRooms.on('disconnect', async () => {
  for (k in vueApp.users) {  //关闭所有媒体流
    let user = vueApp.users[k]
    if (user.producer.video) transports.close(user.producer.video.id)
    if (user.producer.desktop) transports.close(user.producer.desktop.id)
    if (user.producer.audio) transports.close(user.producer.audio.id)
    delete vueApp.users[k]
  }
})

ioRooms.on('join', async (req) => {
  console.log('join', req)

  req.stream = new MediaStream
  req.primary = false
  req.muted = false

  Vue.set(vueApp.users, req.userId, req)
})

ioRooms.on('leave', async (req) => {
  console.log('leave:', req)
  if (req.producer.video) await meeting.close(req.producer.video)
  if (req.producer.audio) await meeting.close(req.producer.audio)
  if (req.producer.desktop) await meeting.close(req.producer.desktop)
  Vue.delete(vueApp.users, req.userId)
})

ioRooms.on('produce', async (req, res) => {
  console.log('produce', req)

  if (req.userId == sock.userId)  //自己
    return

  vueApp.users[req.userId].producer = req.producer

  let stream = req.stream
  if (req.producer.video) {
  }

  if (req.producer.desktop) {
  }

  if (req.producer.audio) {
  }

  async function updateUserList(params) {
    // let video = vueApp.$refs[req.userId].srcObject = await stream
    let video = document.querySelector(`#video-${params.userId}`)
    if (video) video.srcObject = await stream
    else setTimeout(updateUserList, 1000, params)
  }

  setTimeout(updateUserList, 1000, params)
})