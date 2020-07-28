const ioRooms = require('socket.io-client')('https://dungbeetles.xyz:8000')
ioRooms.request = require('./lib/socket.io-promise').promise(ioRooms)
const Vue = require('vue')

ioRooms.on('connect', () => { })
ioRooms.on('disconnect', () => { })

let vueRoom = new Vue({
  el: '#room',
  data: {
    roomId: ''
  },
  methods: {
    enterRoom: async function () {
      if ('' == this.roomId) {
        alert('enter the room id!')
        return
      }

      window.localStorage.setItem('userId', ioRooms.id)
      window.localStorage.setItem('roomId', this.roomId)
      window.localStorage.setItem('nickname', vueApp.nickname)

      $('#room').modal('hide')
      window.location.href = 'meeting.html'
    }
  }
})

var vueApp = new Vue({
  el: '#app',
  data: {
    nickname: 'rookie',
    microphoneActivable: true,
    cameraActivable: true
  },
  methods: {
    exitApp: () => {
      window.close()
    },
    newMeeting: async () => {
      vueRoom.roomId = ''
      $('#room-id')[0].readOnly = true

      let res = await ioRooms.request('newRoom')

      if (res.err) {
        alert(res.err)
        return
      }

      vueRoom.roomId = res.val
    },
    joinMeeting: async () => {
      $('#room-id')[0].readOnly = false
      $('#room').on('shown.bs.modal', () => $('#room-id').trigger('focus'))
    }
  }
})

async function testDevice() {
  var primary = document.querySelector('#primary')
  let userStream = new MediaStream

  try {
    stream = await navigator.mediaDevices.getUserMedia({ audio: true })
    userStream.addTrack(stream.getAudioTracks()[0])
  } catch (e) {
    alert('failed to open the audio device!')
  }

  try {
    stream = await navigator.mediaDevices.getUserMedia({ video: true })
    userStream.addTrack(stream.getVideoTracks()[0])
  } catch (e) {
    alert('failed to open the video device!')
  }

  // stream = await navigator.mediaDevices.getDisplayMedia({ video: true })
  // userStream.addTrack(stream.getVideoTracks()[0])

  primary.srcObject = await userStream
}

testDevice()