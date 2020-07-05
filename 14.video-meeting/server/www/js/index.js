const Vue = require('vue')
const axios = require('axios')

var vueApp = new Vue({
  el: '#app',
  data: {
    nickname: '',
    roomId: '',
    microphoneActivable: false,
    cameraActivable: false,
  },
  methods: {
    microphoneSwitch: function () {
      this.microphoneActivable = !this.microphoneActivable
    },
    cameraSwitch: function () {
      this.cameraActivable = !this.cameraActivable
    },

    quit: function () {
      if (!confirm("确定退出吗？"))
        return

      alert('退出！')
    },
    createMeeting: async function () {
      alert('')
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
      alert('join')
    },
  }
})