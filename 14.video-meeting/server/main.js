var a = new Date
var b = new Date

var h = a.getFullYear()
a.setFullYear(h - 3)

console.log(a,b)

var s = b - a


console.log(new Date(s))