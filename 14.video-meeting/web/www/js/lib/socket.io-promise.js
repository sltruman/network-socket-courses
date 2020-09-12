// Adds support for Promise to socket.io-client
exports.promise = function (socket) {
  return function request(type, data = {}) {
    return new Promise((resolve, reject) => {
      socket.emit(type, data, resolve)
    })
  }
}