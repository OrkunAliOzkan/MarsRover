const http = require('http');

const express = require('express');
const app = express();

const server = http.createServer(app);
app.use(express.static(`${__dirname}`));

const socketio = require('socket.io');
const io = socketio(server); 

var rx = 0;
var ry = 0;
var rangle = 0;

io.on('connection', (sock) => {
    console.log("Client connected");

    sock.on('update', (data) => {
        console.log('update received');
        console.log(data);
        rx = data.rover_x;
        ry = data.rover_y;
        rangle = data.rover_angle;
    });

});

app.get('/rover_test', (req, res) => {
    res.writeHead(200, {'Content-Type':'application/json'})
    const data = JSON.stringify(
        {
            x: rx,
            y: ry,
            angle: rangle
        }
    )
    res.end(data)
})

server.on('error', (err) => {
    console.error(err)
});

server.listen(3000,'0.0.0.0', () => {
    console.log('Server is running on port 3000')
});

