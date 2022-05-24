var express = require('express');
var server = express();

var htmlPath = 'D:/User_Files/Summer_Project/MarsRover/javascript-tut/local_proc_server/index.html';
var jsPath = 'D:/User_Files/Summer_Project/MarsRover/javascript-tut/local_proc_server/primeOrNot.js';

server.get('/', function(req, res) {
    res.sendFile(htmlPath);
});

server.get('/primeOrNot.js', function(req, res){
    console.log("requested javascript");
    res.sendFile(jsPath);
});

console.log('Server is running on port 3000');
server.listen(3000,'127.0.0.1');