const http = require('http');

const express = require('express');
const app = express();

const server = http.createServer(app);
app.use(express.static(`${__dirname}`));
// app.use(express.static(`${__dirname}/../client/css`));
// app.use(express.static(`${__dirname}/../client/images`));

// // method 2
// const {spawn} = require('child_process');
// const python = spawn('python', ['map_sim.py']);
// python.stdout.on('data', (data) => {
//     console.log(`Python printed this to console: ${data}`);
// })
// python.on('close', (code) => {
//     console.log(`Python process exited with code ${code}`)
// })

var current_console = 0;
// 0 - javascript printed last
// 1 - python printed last

// function smart_log(input, id) {
//     if (current_console != id) {
//         return
//     } 
// }

app.get("/", (req, res) => {
    res.sendFile(`${__dirname}/client/html/home.html`)
})

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

server.on('error', (err) => {
    console.error(err)
});

server.listen(3000,'0.0.0.0', () => {
    console.log('Server is running on port 3000')
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

