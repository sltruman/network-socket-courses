const Vue = require('vue')
const axios = require('axios')

var vueApp = new Vue({
  el: '#app',
  data: {
    nickname: '张三',
    microphoneActivable: false,
    cameraActivable: true,
  },
  methods: {
    microphoneSwitch: function () {
      this.microphoneActivable = !this.microphoneActivable
      testDevice()
    },
    cameraSwitch: function () {
      this.cameraActivable = !this.cameraActivable
      testDevice()
    },

    quit: function () {
      if (!confirm("确定退出吗？"))
        return
    },
    createMeeting: async function () {
      var res = await axios.get('https://dungbeetles.xyz:8000/newRoom')

      var ret = res.data
      if (ret.err) {
        console.error(ret.err)
        return
      }

      vueEnterRoom.roomId = ret.val
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
      var res = await axios.get('https://dungbeetles.xyz:8000/joinRoom', {
        params: { roomId: this.roomId, nickname: vueApp.nickname }
      })

      var ret = res.data

      if (ret.err) {
        console.error(ret.err)
        return
      }

      window.localStorage.setItem('server', ret.val.server)
      window.location.href = '/meeting.html'
    }
  }
})

async function testDevice() {
  var stream = null
  var camera = document.querySelector('#camera')

  if (vueApp.cameraActivable || vueApp.microphoneActivable) {
    stream = await navigator.mediaDevices.getUserMedia({ video: vueApp.cameraActivable, audio: vueApp.microphoneActivable })
    camera.srcObject = await stream
  } else {
    camera.srcObject = null
  }
}

testDevice()