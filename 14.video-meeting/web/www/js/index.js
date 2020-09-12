const ioRooms = require('socket.io-client')('https://192.168.16.224:8000')
ioRooms.request = require('./lib/socket.io-promise').promise(ioRooms)
const Vue = require('vue')
const DecibelMeter = require('decibel-meter')

let vueRoom = new Vue({
  el: '#room',
  data: {
    roomId: ''
  },
  methods: {
    enterRoom: async function () {
      if (!ioRooms.connected) {
        alert('failed to connect room service!')
        return
      }

      if ('' == this.roomId) {
        alert('enter the room id!')
        return
      } 

      $('#room').modal('hide')
      window.location.href = `/meeting.html?userId=${ioRooms.id}&roomId=${vueRoom.roomId}&nickname=${vueApp.nickname}`
    }
  }
})

var vueApp = new Vue({
  el: '#app',
  data: {
    nickname: 'user ' + Math.round(Math.random() * 100),
    status: 'connecting',
    userStream: new MediaStream,
    primary: document.querySelector('#primary'),
    meter: new DecibelMeter('meter')
  },
  methods: {
    microphoneSwitch: async () => {
      //判断麦克风按钮状态
      if ($('#microphoneSwitch')[0].classList.contains('active')) {
        vueApp.meter.on('disconnect', (meter) => { $('#microphoneLevel')[0].style.height = '0%' })
        vueApp.meter.disconnect()
      } else {
        vueApp.meter.listenTo(0, (dB, percent, level) => $('#microphoneLevel')[0].style.height = percent + '%').catch(err => alert(err))
      }

      $('#microphoneSwitch').toggleClass('active')
    },
    cameraSwitch: async () => {
      try {
        if ($('#cameraSwitch')[0].classList.contains('active')) {
          vueApp.userStream.removeTrack(vueApp.userStream.video)
        } else {
          let stream = await navigator.mediaDevices.getUserMedia({
            video: {
              width: {
                min: 1280,
                ideal: 1920,
                max: 2560,
              },
              height: {
                min: 720,
                ideal: 1080,
                max: 1440,
              },
              facingMode: "user"
            }
          })

          let track = stream.getVideoTracks()[0]
          vueApp.userStream.video = track
          vueApp.userStream.addTrack(track)
        }

        vueApp.primary.srcObject = await vueApp.userStream
        $('#cameraSwitch').toggleClass('active')
      } catch (e) {
        alert('failed to open the video device!')
      }
    },
    exitApp: () => {
      window.close()
    },
    newMeeting: async () => {
      vueRoom.roomId = ''
      $('#room-id')[0].readOnly = true
      vueRoom.roomId = Math.round(Math.random() * 1000000)
    },
    joinMeeting: async () => {
      $('#room-id')[0].readOnly = false
      $('#room').on('shown.bs.modal', () => $('#room-id').trigger('focus'))
    }
  }
})

ioRooms.on('connect', () => {
  vueApp.status = 'connected'
})

ioRooms.on('connect_error', (obj) => {
  vueApp.status = 'connect error'
})

ioRooms.on('disconnect', () => {
  vueApp.status = 'disconnected'
})
