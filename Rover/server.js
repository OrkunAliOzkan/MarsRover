var express = require('express');
var server = express();

server.use(express.static(__dirname ));

server.get('/', function(req, res) {
    res.sendFile('/home/ubuntu/website/html/home.html');
});

server.get('/home.html', function(req, res) {
    res.sendFile('/home/ubuntu/website/html/home.html');
});

server.get('/rover.html', function(req, res) {
    res.sendFile('/home/ubuntu/website/html/rover.html');
});

server.get('/members.html', function(req, res) {
    res.sendFile('/home/ubuntu/website/html/members.html');
});

server.get('/specification.html', function(req, res) {
    res.sendFile('/home/ubuntu/website/html/specification.html');
});


console.log('Server is running on port 3000');
server.listen(3000,'0.0.0.0');