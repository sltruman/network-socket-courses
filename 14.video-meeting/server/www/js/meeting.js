const ioRooms = require('socket.io-client')('https://dungbeetles.xyz:8000')
ioRooms.request = require('./lib/socket.io-promise').promise(ioRooms)
const transports = require('./meeting-transports')
const Vue = require('vue')

let vueMessages = new Vue({
  el: '#messages',
  data: {
    rows: []
  },
  methods: {}
})

var vueApp = new Vue({
  el: '#app',
  data: {
    roomId: '',
    userCount: 0,
    microphoneActivable: false,
    cameraActivable: false,
    screenActivable: false,
    users: {
      // 'user-id': { userId, nickname, producer: { video, audio, desktop }, muted, primary }
    }
  },
  methods: {
    microphoneSwitch: async () => {
      let user = vueApp.users[ioRooms.userId]

      if (this.microphoneActivable) {      //关闭视频
        console.log('close microphone')
        let producerId = user.producer.video.id

        await transports.close(producerId) //停止推送视频流

        let res = await ioRooms.request(`produce`, {
          kind: 'audio',
          producerId: producerId,
          live: false
        })

        if (res.err) {
          alert(res.err)
          return
        }

        $('#microphoneSwitch').toggleClass('active')
        this.microphoneActivable = !this.microphoneActivable
        return
      }


      let audioTrack = null
      try { //获取视频流
        //let stream = await navigator.mediaDevices.getDisplayMedia({ video: true })
        let stream = await navigator.mediaDevices.getUserMedia({ video: false, audio: true })
        audioTrack = stream.getAudioTracks()[0]
      } catch (e) {
        alert('failed to open the microphone!')
        return
      }

      user.stream.addTrack(audioTrack)
      user.stream.audioTrack = videoTrack
      let res = await transports.produce(audioTrack) //推送视频流

      if (res.err) {
        alert(res.err)
        return
      }

      let producerId = res.val
      res = await ioRooms.request(`produce`, {
        kind: 'audio',
        producerId: producerId,
        live: true
      })

      if (res.err) { //遇到错误停止推送
        alert(res.err)
        await transports.close(producerId)
        return
      }


      $('#microphoneSwitch').toggleClass('active')
      this.microphoneActivable = !this.microphoneActivable
    },
    cameraSwitch: async () => {
      let user = vueApp.users[ioRooms.userId]

      if (this.cameraActivable) {      //关闭视频
        console.log('close video')
        let producerId = user.producer.video.id

        await transports.close(producerId) //停止推送视频流

        let res = await ioRooms.request(`produce`, {
          kind: 'video',
          producerId: producerId,
          live: false
        })

        if (res.err) {
          alert(res.err)
          return
        }

        $('#cameraSwitch').toggleClass('active')
        this.cameraActivable = !this.cameraActivable
        return
      }

      let videoTrack = null
      try { //获取视频流
        let stream = await navigator.mediaDevices.getUserMedia({ video: true })
        videoTrack = stream.getVideoTracks()[0]
      } catch (e) {
        alert('failed to open the camera!')
        return
      }

      user.stream.addTrack(videoTrack)
      user.stream.videoTrack = videoTrack
      let res = await transports.produce(videoTrack) //推送视频流

      if (res.err) {
        alert(res.err)
        return
      }

      let producerId = res.val
      res = await ioRooms.request(`produce`, {
        kind: 'video',
        producerId: producerId,
        live: true
      })

      if (res.err) { //遇到错误停止推送
        alert(res.err)
        await transports.close(producerId)
        return
      }


      $('#cameraSwitch').toggleClass('active')
      this.cameraActivable = !this.cameraActivable
    },
    screenSwitch: async () => {
      let user = vueApp.users[ioRooms.userId]

      if (this.screenActivable) {      //关闭视频
        console.log('close desktop')
        let producerId = user.producer.video.id

        await transports.close(producerId) //停止推送视频流

        let res = await ioRooms.request(`produce`, {
          kind: 'video',
          producerId: producerId,
          live: false
        })

        if (res.err) {
          alert(res.err)
          return
        }

        $('#screenSwitch').toggleClass('active')
        this.screenActivable = !this.screenActivable
        return
      }

      let desktopTrack = null
      try { //获取视频流
        let stream = await navigator.mediaDevices.getDisplayMedia({ video: true })
        desktopTrack = stream.getVideoTracks()[0]
      } catch (e) {
        alert('failed to open the desktop!')
        return
      }

      user.stream.addTrack(desktopTrack)
      user.stream.desktopTrack = desktopTrack
      let res = await transports.produce(videoTrack) //推送视频流

      if (res.err) {
        alert(res.err)
        return
      }

      let producerId = res.val
      res = await ioRooms.request(`produce`, {
        kind: 'video',
        producerId: producerId,
        live: true
      })

      if (res.err) { //遇到错误停止推送
        alert(res.err)
        await transports.close(producerId)
        return
      }

      $('#screenSwitch').toggleClass('active')
      this.screenActivable = !this.screenActivable
    },
    sendMessage: async () => { },
    primarySwitch: async (userId) => {
      let u = vueApp.users[userId]
      $('#nickname-primary')[0].innerText = 'current：' + u.nickname
      $('#video-primary')[0].srcObject = await u.stream
    },
    quitMeeting: async () => {
      if (!confirm("Are you sure to quit?"))
        return

      await transports.quit()
      ioRooms.close()
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
  vueApp.roomId = roomId
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

  if (vueApp.users[req.userId]) {
    console.log(`the user '${req.userId}' already exists!`)
    return
  }

  vueMessages.rows.push({ text: `${req.nickname} joins the room!`, lookAt: false })
  Vue.set(vueApp.users, req.userId, req)
  vueApp.userCount++
})

ioRooms.on('leave', async (req) => {
  console.log('leave:', req)
  if (req.producer.video.id) await transports.close(req.producer.video.id)
  if (req.producer.audio.id) await transports.close(req.producer.audio.id)
  if (req.producer.desktop.id) await transports.close(req.producer.desktop.id)
  Vue.delete(vueApp.users, req.userId)
  vueMessages.rows.push({ text: `${req.nickname} leaves the room!`, lookAt: false })
  vueApp.userCount--
})

ioRooms.on('produce', async (req, res) => {
  console.log('produce', req)

  let user = vueApp.users[req.userId] //获取用户资料

  if (req.producer.video.live) {      //用户分享了媒体流
    if (req.userId != ioRooms.userId) {
      let { val, err } = await transports.consume(req.producer.video.id)
      if (err) console.error(err)
      else {
        user.stream.addTrack(val)
        user.stream.videoTrack = val
      }
    }

    vueMessages.rows.push({ text: `${req.nickname} produces the video stream!`, lookAt: true })
  } else {  //用户关闭了视频流
    if (req.userId != ioRooms.userId && req.producer.video.id)
      await transports.close(req.producer.video.id)

    if (user.stream.videoTrack)
      user.stream.removeTrack(user.stream.videoTrack)

    vueMessages.rows.push({ text: `${req.nickname} closes the video stream!`, lookAt: true })
  }

  if (req.producer.audio.live) {      //用户分享了媒体流
    if (req.userId != ioRooms.userId) {
      let { val, err } = await transports.consume(req.producer.audio.id)
      if (err) console.error(err)
      else {
        user.stream.addTrack(val)
        user.stream.audioTrack = val
      }
    }

    vueMessages.rows.push({ text: `${req.nickname} produces the audio stream!`, lookAt: true })
  } else {  //用户关闭了媒体流
    if (req.userId != ioRooms.userId && req.producer.audio.id)
      await transports.close(req.producer.audio.id)

    if (user.stream.audioTrack)
      user.stream.removeTrack(user.stream.audioTrack)

    vueMessages.rows.push({ text: `${req.nickname} closes the audio stream!`, lookAt: true })
  }

  user.producer = req.producer //更新推送信息

  await Vue.nextTick(async () => {
    $(`#video-${user.userId}`)[0].srcObject = await user.stream
    if (user.userId == req.userId) //如果是自己则静音
      $(`#video-${user.userId}`)[0].muted = true
  })
})