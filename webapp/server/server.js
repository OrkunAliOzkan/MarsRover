const http = require('http');

const express = require('express');
const app = express();

const server = http.createServer(app);
app.use(express.static(`${__dirname}/../client`));

const socketio = require('socket.io');
const io = socketio(server); 

// // method 1
// const {PythonShell} = require('python-shell');
// let pyshell = new PythonShell('echo_text.py', { mode: 'text' });
// // let output = "";
// pyshell.stdout.on('data', (data) => {
//     // output += '' + data;
//     console.log(`Python out: ${data}`);
// });

// pyshell.send('hello').send('world').end(function (err) {
//     console.log(output === ('hello' + '\n' + 'world' + '\n'));
//     console.log(output);
// });
// pyshell.on('close', () => console.log("Process ended"));

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
        io.emit('ntop', { x, y });
        // pyshell.send(`W,${x},${y}`);
    });
    
    sock.on('stickmove', ({d, a}) => {
        console.log(`joystick at distance: ${d}, angle: ${a}`)
        // pyshell.send(`J,${d},${a}`);
        io.emit('ntop', {x, y});
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

