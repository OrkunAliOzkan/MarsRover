var express = require('express');
var server = express();

var bodyParser = require('body-parser');
var htmlParser = require('node-html-parser');

server.use(bodyParser.json());
server.use(bodyParser.urlencoded({ extended: true }));

function primeOrNot(num){
    if(num<2)           { return " not a prime"; }
    else if(num==2)     { return " a prime"; }
    else{ 
        const rootnum = Math.sqrt(num);
        for(let d=2; d<rootnum; d++){
            if(num%d==0)        { return " not a prime"; } 
        }
        return " a prime";
    }
}

server.get('/', function(req, res) {
    res.sendFile('/home/ubuntu/website/html/index.html');
});

var fileSystem = require("fs");

server.post('/primality-test', function(req, res) { 
    //formData is a JavaScript object
    const formData = req.body;
    const responseContent = "<p>The number is"+ primeOrNot(formData.num1)+"</p>";
    /*fileSystem.readFile('/Users/sbaig/index.html', 'utf8', function(err,data){ 
        if(err) {
            console.error(err);
            return;
        } 
        return data;
    });*/ 
    const htmlContent = fileSystem.readFileSync('/home/ubuntu/website/html/index.html', 'utf8'); 
    console.log("htmlContent: ", htmlContent);
    
    let htmlTree = htmlParser.parse(htmlContent);
    htmlTree.getElementById("form1").insertAdjacentHTML("afterend",responseContent);
    res.writeHead(200, {'Content-Type':'text/html'});
    res.end(htmlTree.toString()); 
});

console.log('Server is running on port 3000');
server.listen(3000,'0.0.0.0');