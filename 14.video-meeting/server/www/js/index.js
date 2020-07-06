const Vue = require('vue')
const axios = require('axios')

var vueApp = new Vue({
  el: '#app',
  data: {
    nickname: '',
    roomId: '',
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
      return
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
    },
    joinMeeting: async function () {

    },
  }
})

var vueEnterRoom = new Vue({
  el: '#enterRoom',
  data: {
    roomId: ''
  },
  methods: {
    enterRoom: async function () {
      alert('ss')
      window.location.href = '/meeting.html'
    }
  }
})

async function testDevice() {
  var stream = null
  if (vueApp.cameraActivable || vueApp.microphoneActivable) {
    stream = await navigator.mediaDevices.getUserMedia({ video: vueApp.cameraActivable, audio: vueApp.microphoneActivable })
    document.querySelector('#camera').srcObject = await stream
  } else {
    document.querySelector('#camera').stop()
  }
}

// testDevice()