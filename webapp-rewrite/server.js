const http = require('http');

const express = require('express');
const app = express();

const server = http.createServer(app);

(() => {
    app.use(express.static(`${__dirname}/client`));

    app.get('/', function(req, res) {
        res.sendFile(`${__dirname}/html/home.html`);
    });

    app.get('/home.html', function(req, res) {
        res.sendFile(`${__dirname}/html/home.html`);
    });

    app.get('/rover.html', function(req, res) {
        res.sendFile(`${__dirname}/html/rover.html`);
    });

    app.get('/members.html', function(req, res) {
        res.sendFile(`${__dirname}/html/members.html`);
    });

    app.get('/specification.html', function(req, res) {
        res.sendFile(`${__dirname}/html/specification.html`);
    });

    app.on('error', (err) => {
        console.error(err)
    });

    app.listen(3000,'0.0.0.0', () => {
        console.log('Server is running on port 3000')
    });
})();

const {spawn} = require('child_process');
const map_sim = spawn('python', ['map_sim.py']);
map_sim.stdout.on('data', (data) => {
    console.log(`Python printed this to console: ${data}`);
})
map_sim.on('close', (code) => {
    console.log(`Simulation ended with code ${code}`)
})

var current_console = 0;
// 0 - javascript printed last
// 1 - python printed last

// function smart_log(input, id) {
//     if (current_console != id) {
//         return
//     } 
// }

const socketio = require('socket.io');
const io = socketio(server); 

io.on('connection', (sock) => {
    console.log("Client connected");
    io.emit('update', data);

    sock.on('waypoint', ({ x, y }) => {
        io.emit('waypoint', { x, y });
        console.log(`waypoint placed at x: ${x}, y: ${y}`);
        // pyshell.send(`W,${x},${y}`);
    });
    
});

const data = {
    "rover": {
        "posX": 100,
        "posY": 100,
        "angle": 75  
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

