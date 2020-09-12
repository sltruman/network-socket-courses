browserify www/js/index.js -o www/js/index.bundle.js
browserify www/js/meeting.js -o www/js/meeting.bundle.js
http-server -S -C ./certs/ca.crt -K ./certs/ca.key -p 443 www/ 