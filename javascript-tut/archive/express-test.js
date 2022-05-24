var express = require('express');
var server = express();
let htmlContent = `
<!DOCTYPE html>
<html>
 <body>
 <h2>Enter a number to test if it is Prime</h2>
 <form action="/primality-test" method="post">
 <!-- this is a comment: br tag is for line break-->
 <label for="num1">Input number:</label><br>
 <input type="text" id="num1" name="num1"><br>
 <input type="submit" value="Check!">
 </form>
 </body>
</html>
`;

server.get('/', function(req, res) {
    res.writeHead(200, {'Content-Type':'text/html'});
    res.end(htmlContent);
});

server.post('/primality-test', function(req, res) {
    res.writeHead(200, {'Content-Type':'text/html'});
    res.end("<html><body>Form submitted...</body></html>");
});

console.log('Server is running on port 3000');
server.listen(3000,'127.0.0.1');