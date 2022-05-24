var express = require('express');
var server = express();

var htmlPath = '~/MarsRover/javascript-tut/local_proc_server/index.html';
var jsPath = '~/MarsRover/javascript-tut/local_proc_server/primeOrNote.js';

server.get('/', function(req, res) {
    res.sendFile(htmlPath);
});

server.get('/primeOrNot.js', function(req, res){
    console.log("requested javascript");
    res.sendFile(jsPath);
});

console.log('Server is running on port 3000');
server.listen(3000,'0.0.0.0');