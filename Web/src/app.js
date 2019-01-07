var app = new Vue({
  el: '#app',
  data: {
    message: 'Welcome to Giant Company File Center!',
    uuid: '',
    sessionInfo: '',
    username: '',
    veriryButtonStatus: 'btn-primary',
    veriryButtonText: 'Verify',
    isVeriryButtonDisabled: false,
    filePath: '',
    downloadVerified: false
  },
  methods: {
    generateSession: function () {
      if (this.veriryButtonStatus != 'btn-success') {
        axios.get("http://127.0.0.1:8080/get/uuid?username=" + this.username)
          .then(response => {
            this.uuid = response.data
            this.sessionInfo = "Session UUID: " + this.uuid
          })
        this.startMonitor()
        this.veriryButtonStatus = 'btn-warning'
        this.veriryButtonText = 'Waiting validation.'
        this.isVeriryButtonDisabled = true
      }
    },
    startMonitor: function () {
      window.setInterval(() => {
        setTimeout(() => {
          axios.get("http://127.0.0.1:8080/get/status?uuid=" + this.uuid)
            .then(response => {
              if (response.data != "waiting") {
                this.veriryButtonStatus = 'btn-success'
                this.isVeriryButtonDisabled = false
                this.veriryButtonText = 'Verified'
                this.message = "verifed"

                this.downloadVerified = true
                this.filePath = response.data
              }
              this.message = response.data
            })
        }, 0)
      }, 300)
    }
  },
})