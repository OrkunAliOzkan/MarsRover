var express = require('express');
var server = express();

console.log(__dirname);

server.use(express.static(__dirname ));

// const path = require('path');
// const htmlPath = path.resolve('../client/index.html');

server.get('/', function(req, res) {
    res.sendFile(`${__dirname}/html/home.html`);
});

server.get('/home.html', function(req, res) {
    res.sendFile(`${__dirname}/html/home.html`);
});

server.get('/rover.html', function(req, res) {
    res.sendFile(`${__dirname}/html/rover.html`);
});

server.get('/members.html', function(req, res) {
    res.sendFile(`${__dirname}/html/members.html`);
});

server.get('/specification.html', function(req, res) {
    res.sendFile(`${__dirname}/html/specification.html`);
});


console.log('Server is running on port 3000');
server.listen(3000,'0.0.0.0');