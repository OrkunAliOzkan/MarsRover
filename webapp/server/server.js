const http = require('http');

const express = require('express');
const app = express();

const server = http.createServer(app);
app.use(express.static(`${__dirname}/../client`));

const socketio = require('socket.io');
const io = socketio(server); 

io.on('connection', (sock) => {
    console.log("Client connected");

    sock.on('waypoint', ({ x, y }) => {
        io.emit('waypoint', { x, y });
        console.log(`waypoint placed at x: ${x}, y: ${y}`);
    });
    
    sock.on('stickmove', ({d, a}) => {
        console.log(`joystick at distance: ${d}, angle: ${a}`)
    })
});

server.on('error', (err) => {
    console.error(err)
});

server.listen(3000,'0.0.0.0', () => {
    console.log('Server is running on port 3000')
});

// method 1
// const {PythonShell} = require('python-shell');

// PythonShell.run('map_test.py', null, (err) => {
//     if (err) throw err;
//     console.log("finished");
// })

const {spawn} = require('child_process');
const python = spawn('python', ['map_sim.py']);
python.stdout.on('data', (data) => {
    console.log(`Python printed this to console: ${data}`);
})
python.on('close', (code) => {
    console.log(`Python process exited with code ${code}`)
})