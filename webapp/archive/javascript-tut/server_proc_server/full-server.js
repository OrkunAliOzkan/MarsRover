var express = require('express');
var server = express();

var bodyParser = require('body-parser');
var htmlParser = require('node-html-parser');

server.use(bodyParser.json());
server.use(bodyParser.urlencoded({ extended: true }));

var fileSystem = require('fs');

var htmlPath = 'D:/User_Files/Summer_Project/MarsRover/javascript-tut/index.html';
server.get('/', function(req, res) {
    res.sendFile(htmlPath);
});

function primeOrNot(num){
    if(num<2){
        return 1; 
    } else if(num==2){
        return -1;
    } else{
        const rootnum = Math.sqrt(num);
        for (let d=2; d<rootnum; d++) {
            if(num%d==0){
                return d;
            }
        }
        return -1;
    }
}

server.post('/primality-test', function(req, res) {
    //formData is a JavaScript object
    const formData = req.body;

    // logic
    const prime_result = primeOrNot(formData.num1);
    let p_or_not;
    let factor_info = "";
    if (prime_result === -1) {
        p_or_not = " a prime ";
    } else {
        p_or_not = " not a prime ";
        factor_info = "<p>" + prime_result + " is a factor</p>"
    }
    const responseContent = "<p>The number is"+ p_or_not +"</p>" + factor_info;

    /*fileSystem.readFile('/Users/sbaig/index.html', 'utf8', function(err,data){
    if(err){
    console.error(err);
    return;
    }
    return data;
    });*/

    // html processing
    const htmlContent = fileSystem.readFileSync(htmlPath, 'utf8');
    console.log("htmlContent: ", htmlContent);
    let htmlTree = htmlParser.parse(htmlContent);
    htmlTree.getElementById("form1").insertAdjacentHTML("afterend",responseContent);
    res.writeHead(200, {'Content-Type':'text/html'});
    res.end(htmlTree.toString());
});

console.log('Server is running on port 3000');
server.listen(3000,'127.0.0.1');