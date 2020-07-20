// Adds support for Promise to socket.io-client
exports.promise = function (socket) {
  return function request(type, data = {}) {
    return new Promise((resolve, reject) => {
      setTimeout(function () {
        resolve({ err: 'timeout of network reqeust!', val: null })
      }, 5000)

      socket.emit(type, data, resolve)
    })
  }
}