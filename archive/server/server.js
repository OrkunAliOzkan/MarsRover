const http = require('http');

const express = require('express');
const app = express();

const server = http.createServer(app);
app.use(express.static(`${__dirname}/../client`));

const socketio = require('socket.io');
const io = socketio(server); 

// // method 2
// const {spawn} = require('child_process');
// const python = spawn('python', ['map_sim.py']);
// python.stdout.on('data', (data) => {
//     console.log(`Python printed this to console: ${data}`);
// })
// python.on('close', (code) => {
//     console.log(`Python process exited with code ${code}`)
// })

io.on('connection', (sock) => {
    console.log("Client connected");

    sock.on('waypoint', ({ x, y }) => {
        io.emit('waypoint', { x, y });
        console.log(`waypoint placed at x: ${x}, y: ${y}`);
        io.emit('ntop', `W,${x},${y}`);
        // pyshell.send(`W,${x},${y}`);
    });
    
    sock.on('stickmove', ({ x, y }) => {
        console.log(`joystick at x: ${x}, y: ${y}`)
        io.emit('ntop', `J,${x},${y}`);
        // pyshell.send(`J,${d},${a}`);
    });

    sock.on('pton', (s) => {
        console.log(`Python out: ${s}`);
    });
});

server.on('error', (err) => {
    console.error(err)
});

server.listen(3000,'0.0.0.0', () => {
    console.log('Server is running on port 3000')
});

