const http = require('http');
const express = require('express');
const socketio = require('socket.io');

const app = express();
const server = http.createServer(app);
const io = socketio(server); 

// const path = require('path');
// const htmlPath = path.resolve('../client/index.html');

io.on('connection', (sock) => {
    console.log("Client connected")
});

app.use(express.static(`${__dirname}/../client`));

// app.get('/', function(req, res) {
//     res.sendFile(htmlPath);
// });

server.on('error', (err) => {
    console.error(err)
});

server.listen(3000,'0.0.0.0', () => {
    console.log('Server is running on port 3000')
});
