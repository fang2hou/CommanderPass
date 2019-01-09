// API site
const config = {
  timeout: 500,
  baseURL: 'http://127.0.0.1:8080'
}
httpRequest = axios.create(config);

var RCAlert = new Vue({
  el: "#rc-alert",
  data: {
    isShown: true,
  },
  methods: {
    closeSelf: function () {
      this.isShown = false;
    },
  },
});

var RCMain = new Vue({
  el: "#rc-main-container",
  data: {
    uuid: '',
    username: '',
    isVeriryButtonDisabled: false,
    searchButton: {
      isShown: true,
      status: 'btn-primary',
      text: 'Search',
    },
    downloadButton: {
      path: '',
      isShown: false,
    },
    inputedFileName: '',
    fileList: [],
  },
  methods: {
    startMonitor: function () {
      window.setInterval(() => {
        setTimeout(() => {
          httpRequest.get("/get/status?uuid=" + this.uuid).then(response => {
            if (response.data == "waiting") { return; }
            this.searchButton.status = 'btn-success'
            this.searchButton.text = 'Verified'
            this.searchButton.isShown = false

            this.downloadButton.path = response.data
            this.downloadButton.isShown = true;
          })
        }, 0)
      }, 300)
    },
    searchFileByName: function () {
      httpRequest.get('/get/filelist?search=' + this.inputedFileName).then(response => {
        this.fileList = response.items[0]
      })
    },
    generateSession: function () {
      if (this.searchButton.status != 'btn-success') {
        httpRequest.get("/get/uuid?username=" + this.inputedFileName).then(response => {
          this.uuid = response.data;
        })
        this.startMonitor();
        this.searchButton.status = 'btn-warning';
        this.searchButton.text = 'Waiting validation.';
      }
    },
  }
});

// Animation
$('.ad .letter').each(function () {
  $(this).html($(this).text().replace(/([^\x00-\x80]|\w)/g, "<span class='letter'>$&</span>"));
});

anime.timeline({ loop: true })
  .add({
    targets: '.ad .letter',
    translateY: ["1.6em", 0],
    translateZ: 0,
    duration: 1000,
    delay: function (el, i) {
      return 50 * i;
    }
  }).add({
    targets: '.ad',
    opacity: 0,
    duration: 500,
    easing: "easeOutExpo",
    delay: 500
  }).add({
    targets: '.ad .line',
    scaleX: [0, 1],
    opacity: [0.5, 1],
    easing: "easeOutExpo",
    duration: 700,
    offset: '-=875',
    delay: function (el, i, l) {
      return 80 * (l - i);
    }
  });