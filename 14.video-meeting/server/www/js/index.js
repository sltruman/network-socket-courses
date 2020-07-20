const sock = require('socket.io-client')('https://dungbeetles.xyz:8000', {
  reconnection: true,             // whether to reconnect automatically
  reconnectionAttempts: Infinity, // number of reconnection attempts before giving up
  reconnectionDelay: 1000,        // how long to initially wait before attempting a new reconnection
  reconnectionDelayMax: 5000,     // maximum amount of time to wait between reconnection attempts. Each attempt increases the reconnection delay by 2x along with a randomization factor
  randomizationFactor: 0.5
})

sock.request = require('./lib/socket.io-promise').promise(sock)
const meeting = require('./index-meeting')

const Vue = require('vue')

var vueApp = new Vue({
  el: '#app',
  data: {
    isMeeting: false,
    nickname: 'rookie',
    microphoneActivable: false,
    cameraActivable: false,
    screenActivable: false,
    microphoneTestActivable: true,
    cameraTestActivable: true,
    users: {
      // 'user-id': { userId, nickname, producer: { video, audio, desktop }, muted, primary }
    }
  },
  methods: {
    microphoneTestSwitch: function () {
      this.microphoneTestActivable = !this.microphoneTestActivable
      testDevice(this.cameraTestActivable, this.microphoneTestActivable)
    },
    cameraTestSwitch: function () {
      this.cameraTestActivable = !this.cameraTestActivable
      testDevice(this.cameraTestActivable, this.microphoneTestActivable)
    },
    microphoneSwitch: async () => {
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
      this.cameraActivable = !this.cameraActivable
      console.log(this.cameraActivable)

      let user = vueApp.users[sock.id]
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
    messageSwitch: function () {

    },
    quit: function () {
      if (!confirm("are you sure to quit?"))
        return

      meeting.quit()
      sock.close()
      window.close()
    },
    newMeeting: async function () {
      if (!sock.connected) {
        alert('failed to connect room server!')
        return
      }

      vueEnterRoom.roomId = ''
      var res = await sock.request('newRoom')

      if (res.err) {
        alert(res.err)
        return
      }

      vueEnterRoom.roomId = res.val
      $('#enterRoom').modal('show')
    },
    joinMeeting: async function () {
      if (!sock.connected) {
        alert('failed to connect room server!')
        return
      }

      $('#enterRoom').modal('show')
    }
  }
})

var vueEnterRoom = new Vue({
  el: '#enterRoom',
  data: {
    roomId: ''
  },
  methods: {
    enterRoom: async function () {
      if ('' == this.roomId) {
        alert('enter the room id!')
        return
      }

      let res = await sock.request('joinRoom', {
        roomId: this.roomId, nickname: vueApp.nickname
      })

      if (res.err) {
        alert(res.err)
        return
      }

      res = await meeting.join(res.val.server.address)

      if (res.err) {
        alert(res.err)
        return
      }

      $('#enterRoom').modal('hide')
      vueApp.isMeeting = true
    }
  }
})

sock.on('connect', () => {

})

sock.on('disconnect', async () => {
  await meeting.quit()
})

sock.on('room', async (req) => {
  console.log('room', req)

  for (id in req) {
    req[id].stream = new MediaStream
    req[id].primary = sock.id == id
    req[id].muted = false
    if (req[id].primary)
      document.querySelector('#primary').srcObject = null
  }

  Vue.set(vueApp, 'users', req)

  for (id in req) {
    await consume(req[id])
  }
})

sock.on('join', async (req) => {
  console.log('join', req)

  if (req.userId == sock.id)  //自己
    return

  req.stream = new MediaStream
  req.primary = false
  req.muted = false

  Vue.set(vueApp.users, req.userId, req)
})

sock.on('produce', async (req, res) => {
  console.log('produce', req)

  if (req.userId == sock.id)  //自己
    return

  vueApp.users[req.userId].producer = req.producer
  await consume(vueApp.users[req.userId])
})

sock.on('pause', async (req) => {
  console.log('pause', req)
  if (req.userId == sock.id) { //自己
    return
  }
})

sock.on('resume', async (req) => {
  console.log('resume', req)
  if (req.userId == sock.id) { //自己
    return
  }
})

sock.on('leave', async (req) => {
  console.log('leave:', req)
  if (req.producer.video) await meeting.close(req.producer.video)
  if (req.producer.audio) await meeting.close(req.producer.audio)
  if (req.producer.desktop) await meeting.close(req.producer.desktop)
  Vue.delete(vueApp.users, req.userId)
})

async function consume(req) {
  let stream = req.stream
  if (req.producer.video) {
    let res = await meeting.consume(req.producer.video)
    let track = res.val
    stream.removeTrack(track)
    stream.addTrack(track)
  } else if (req.producer.desktop) {
    let res = await meeting.consume(req.producer.desktop)
    let track = res.val
    stream.removeTrack(track)
    stream.addTrack(track)
  }

  if (req.producer.audio) {
    let res = await meeting.consume(req.producer.audio)
    let track = res.val
    stream.removeTrack(track)
    stream.addTrack(track)
  }

  async function f(req) {
    // let video = vueApp.$refs[req.userId].srcObject = await stream
    let video = document.querySelector(`#video-${req.userId}`)
    if (video) video.srcObject = await stream
    else setTimeout(f, 1000, req)
  }

  setTimeout(f, 1000, req)
}

async function testDevice(camera, microphone) {
  var stream = null
  var primary = document.querySelector('#primary')

  if (camera || microphone) {
    stream = await navigator.mediaDevices.getUserMedia({ video: camera, audio: microphone })
    primary.srcObject = await stream
    primary.muted = true
  } else {
    primary.srcObject = null
  }
}

testDevice(true, false)