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
        io.emit('ntop', `W,${x},${y}`);
        // pyshell.send(`W,${x},${y}`);
    });
    
    sock.on('stickmove', ({ x, y }) => {
        console.log(`joystick at x: ${x}, y: ${y}`)
        io.emit('ntop', `J,${x},${y}`);
        // pyshell.send(`J,${d},${a}`);
    });

    sock.on('update', (data) => {
        console.log('update received');
        console.log(data);
        rover_data = {
            name: "rover",
            colour: "N",
            pos : {
                x: data.rover_x,
                y: data.rover_y
            },
            angle: data.rover_angle
        }

        alien_data = {
            name: "alien",
            colour: data.alien_colour,
            pos : {
                x: data.rover_x,
                y: data.rover_y
            },
        }

        io.emit('rover-update', rover_data);
        io.emit('alien-update', alien_data);
    });

});

server.on('error', (err) => {
    console.error(err)
});

server.listen(3000,'0.0.0.0', () => {
    console.log('Server is running on port 3000')
});

