var http = require('http');
var server = http.createServer(function(req, res){
    let htmlContent = "";
    res.writeHead(200, {'Content-Type':'text/html'})
    res.end(htmlContent);
    console.log("served")
});
console.log('Server is running on port 20000');
server.listen(3000,'0.0.0.0');