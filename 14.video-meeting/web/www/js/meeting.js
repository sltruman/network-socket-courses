const qs = require("querystring")

//userId=sU5pRDdx_-VZ1JeLAAAc&roomId=596346&nickname=%E5%BC%A0%E4%B8%89
const { userId, roomId, nickname } = qs.parse(window.location.search.substring(1))
if (userId == undefined || roomId == undefined)
  window.location.href = '/index.html'

const Vue = require('vue')

let vueMessages = new Vue({
  el: '#messages',
  data: {
    rows: []
  },
  methods: {

  }
})

var vueApp = new Vue({
  el: '#app',
  data: {
    status: 'connecting',
    roomId: roomId,
    userCount: 0,
    users: {}
  },
  methods: {
    microphoneSwitch: async () => {
      let user = vueApp.users[userId]

      if ($('#microphoneSwitch')[0].classList.contains('active')) {
        transports.remove(user.stream.audio)
        user.stream.removeTrack(user.stream.audio)
        updateProducerId(null, 'audio')
      } else {
        try { //获取视频流
          let stream = await navigator.mediaDevices.getUserMedia({ audio: true })
          user.stream.audio = stream.getAudioTracks()[0]
          user.stream.addTrack(user.stream.audio)
          transports.produce(user.stream.audio, ({ val, err }) => {
            user.producer.audio = val
            updateProducerId(val, 'audio')
          })
        } catch (e) {
          alert(e)
          return
        }
      }

      $('#microphoneSwitch').toggleClass('active')
    },
    cameraSwitch: async () => {
      let user = vueApp.users[userId]

      if ($('#cameraSwitch')[0].classList.contains('active')) {
        transports.remove(user.stream.video)
        user.stream.removeTrack(user.stream.video)
        updateProducerId(null, 'video')
      } else {
        try { //获取视频流
          let stream = await navigator.mediaDevices.getUserMedia({ video: true })
          user.stream.video = stream.getVideoTracks()[0]
          user.stream.addTrack(user.stream.video)
          transports.produce(user.stream.video, ({ val, err }) => {
            user.producer.video = val
            updateProducerId(val, 'video')
          })
        } catch (e) {
          alert(e)
          return
        }
      }

      $('#cameraSwitch').toggleClass('active')
    },
    screenSwitch: async () => {
      let user = vueApp.users[userId]

      if ($('#screenSwitch')[0].classList.contains('active')) {
        transports.remove(user.stream.display)
        user.stream.removeTrack(user.stream.display)
        updateProducerId(null, 'display')
      } else {
        try { //获取视频流
          let stream = await navigator.mediaDevices.getDisplayMedia({ video: true })
          user.stream.display = stream.getVideoTracks()[0]
          user.stream.addTrack(user.stream.display)
          transports.produce(user.stream.display, ({ val, err }) => {
            user.producer.display = val
            updateProducerId(val, 'display')
          })
        } catch (e) {
          alert(e)
          return
        }
      }

      $('#screenSwitch').toggleClass('active')
    },
    sendMessage: async () => { },
    primarySwitch: async (userId) => {
      let u = vueApp.users[userId]
      $('#nickname-primary')[0].innerText = u.nickname
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

const transports = require('./meeting-transports')
const ioRooms = require('socket.io-client')('https://192.168.16.224:8000')
ioRooms.request = require('./lib/socket.io-promise').promise(ioRooms)

ioRooms.on('connect', () => {
  vueApp.status = 'connected'
  authorize()
})

ioRooms.on('disconnect', async () => {
  vueApp.status = 'disconnect'
  if ($('#screenSwitch')[0].classList.contains('active')) {
    vueApp.screenSwitch()
  }
})

function authorize() {   //用户认证 
  ioRooms.emit(`authorize`, { userId: userId }, (res) => {
    if (res.err) {
      feebackError(res.err)
      return
    }

    joinRoom()
  })
}

let transportsInitialized = false
async function joinRoom() {
  ioRooms.on('join', onUserJoinedEvent)
  ioRooms.on('leave', onUserLeftEvent)
  ioRooms.on('produce', onUserProducedEvent)

  let askingUsers = []
  for (k in vueApp.users) askingUsers.push(k)

  let { val, err } = await ioRooms.request('joinRoom', { roomId: roomId, nickname: nickname, askingUsers: askingUsers })
  if (err) {
    feedbackError()
    return
  }

  if (!transportsInitialized) {
    await transports.connect(val.server.address) //连接传送管理服务
    transportsInitialized = true
  }
}

function onUserJoinedEvent(req) {
  // req = {
  //   userId: 'user-id-1', roomId: 'room-id-1', nickname: '张三', server: { address: 'dungbeetles.xyz:8001' },
  //   producer: {
  //     video: '',
  //     display: '',
  //     audio: ''
  //   },
  //   stream: null,
  //   primary: false,
  //   muted: false
  // }

  if (vueApp.users[req.userId]) return //如果已存在则什么也不干
  req.stream = new MediaStream
  req.primary = false
  req.muted = (req.userId == userId)

  vueMessages.rows.push(`${req.nickname} joined the room!`)
  Vue.set(vueApp.users, req.userId, req)
  vueApp.userCount++

  if (req.producer.video)
    onUserProducedEvent({ userId: req.userId, kind: 'video', producerId: req.producer.video })
  if (req.producer.audio)
    onUserProducedEvent({ userId: req.userId, kind: 'audio', producerId: req.producer.audio })
  if (req.producer.display)
    onUserProducedEvent({ userId: req.userId, kind: 'display', producerId: req.producer.display })
}

async function onUserLeftEvent(req) {
  console.log('leave:', req)
  let user = vueApp.users[req.userId] //获取用户资料

  if (user.stream.video) transports.remove(user.stream.video)
  if (user.stream.audio) transports.remove(user.stream.audio)
  if (user.stream.display) transports.remove(user.stream.display)

  Vue.delete(vueApp.users, req.userId)
  vueMessages.rows.push(`${user.nickname} left the room!`)
  vueApp.userCount--
}

// { userId:'', kind:'', producerId:null|'??' }
async function onUserProducedEvent(req) {
  console.log('produce', req)

  let user = vueApp.users[req.userId] //获取用户资料

  vueMessages.rows.push(`${user.nickname} ${req.producerId ? 'produces' : 'closes'} the ${req.kind} stream!`)

  if (req.userId != userId) {
    user.producer[req.kind] = req.producerId

    if (req.producerId) { //拉取
      transports.consume(req.producerId, ({ val, err }) => {
        console.log(val)
        if (err) console.error(err)
        else {
          user.stream.display = val
          user.stream.addTrack(val)
        }
      })
    } else { //移除
      transports.remove(user.stream.display)
      user.stream.removeTrack(user.stream.display)
    }
  }

  await Vue.nextTick(async () => {  //更新界面
    $(`#video-${user.userId}`)[0].srcObject = await user.stream
    $(`#video-${user.userId}`)[0].muted = user.muted
  })
}

function feebackError(msg) {
  ioRooms.disconnect()
  alert(res.err)
  window.location.href = 'index.html' //返回准备界面
}

async function updateProducerId(id, kind) {
  let { val, err } = await ioRooms.request(`produce`, {
    kind: kind,
    producerId: id
  })

  if (err) feebackError(res.err)
}