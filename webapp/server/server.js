//--------Utility Functions--------//
var log_state = 0
// 0 - javascript last printed, 1 - python last printed

function print(output, id) {
    if (id != log_state) {
        log_state = id;
        const name = id == 0 ? "Nodejs" : "Python";
        console.log(`<--------${name} Console-------->`);
    } 
    console.log(output);
}

//--------Initialising Server--------//
const http = require('http');

const express = require('express');
const app = express();

const server = http.createServer(app);
app.use(express.static(`${__dirname}/../client`));

server.on('error', (err) => {
    console.error(err)
});

server.listen(3000,'0.0.0.0', () => {
    print('Server is running on port 3000', 0)
});

//--------Initialising Python Control Script--------//
const {spawn} = require('child_process');
const control_script = spawn('python', ['main.py']);
print("Control Script started", 0)

control_script.stdout.on('data', (data) => {
    print(`${data}`, 1);
})
control_script.on('close', (code) => {
    print(`Simulation ended with code ${code}`, 0)
})

//--------Initialising Socket.io (client-side communication)--------//
const socketio = require('socket.io');
const io = socketio(server); 

io.on('connection', (sock) => {
    print("Client connected", 0);

    sock.on('waypoint', ({ x, y }) => {
        io.emit('waypoint', { x, y });
        print(`waypoint placed at x: ${x}, y: ${y}`, 0);
        control_script.stdin.write(`W,${x},${y}\n`);
    });

});

var state = {
    "rover": {
        "posX": 200,
        "posY": 200,
        "angle": 180  
    },
    "alien": [
        {
            "colour": "red",
            "posX": 50,
            "posY": 70
        },
        {
            "colour": "blue",
            "posX": 150,
            "posY": 70
        },
        {
            "colour": "green",
            "posX": 250,
            "posY": 70
        } 
    ],
    "building": [
        {
            "posX": 50,
            "posY": 170
        },
        {
            "posX": 150,
            "posY": 170
        },
        {
            "posX": 250,
            "posY": 170
        } 
    ]
}