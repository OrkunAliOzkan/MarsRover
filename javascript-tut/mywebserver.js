var http = require('http');
var server = http.createServer(function(req,res){
    res.writeHead(200, {"Content-Type":"text/plain"})
    res.end("You have reacher AWS EC2 server");
});

console.log("Server is running on port 3000");
server.listen(3000,'0.0.0.0');