var express = require('express');
var server = express();

// var htmlPath = '/home/ubuntu/MarsRover/javascript-tut/local_proc_server/index.html';
// var jsPath = '/home/ubuntu/MarsRover/javascript-tut/local_proc_server/primeOrNot.js';
var htmlPath = `${__dirname}/index.html`;
var jsPath = `${__dirname}/primeOrNot.js`;

server.get('/', function(req, res) {
    res.sendFile(htmlPath);
});

server.get('/primeOrNot.js', function(req, res){
    console.log("requested javascript");
    res.sendFile(jsPath);
});

console.log('Server is running on port 3000');

server.on('error', (err) => {
    console.error(err)
})

server.listen(3000,'0.0.0.0', () => {
    console.log("Started server")
});

