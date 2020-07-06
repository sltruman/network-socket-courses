const Vue = require('vue')
const axios = require('axios')

var vueMeeting = new Vue({
    el: '#meeting',
    data: {
        microphoneActivable: true,
        cameraActivable: true,
        screenActivable:false,
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
        },
    }
})

