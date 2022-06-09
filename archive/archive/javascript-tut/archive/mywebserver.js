var http = require('http');

let htmlContent = `
<!DOCTYPE html>
<html>
 <body>
 <h2>Enter a number to test if it is Prime</h2>
 <form>
 <!-- this is a comment: br tag is for line break-->
 <label for="num1">Input number:</label><br>
 <input type="text" id="num1" name="num1"><br>
 </form>
 </body>
</html>
 `;

var server = http.createServer(function(req,res){
    res.writeHead(200, {"Content-Type":"text/html"})
    res.end(htmlContent);
});

console.log("Server is running on port 3000");
server.listen(3000,'127.0.0.1')
// server.listen(3000,'0.0.0.0');