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
const http_port = 8000;
const http = require('http');

const express = require('express');
const app = express();

const server = http.createServer(app);
app.use(express.static(`${__dirname}/../client`));

server.on('error', (err) => {
    console.error(err)
});

server.listen(http_port,'0.0.0.0', () => {
    print('Server is running on port 3000', 0)
});

//--------Initialising Socket.io (client-side communication)--------//
const socketio = require('socket.io');
const io = socketio(server); 

// var socketio_list = [];

io.on('connection', (sock) => {
    print("Client connected", 0);
    // socketio_list.push(sock);
    sock.on('waypoint', ({ x, y }) => {
        print(`waypoint placed at x: ${x}, y: ${600 - y}`, 0);

        // map canvas coordinates to arena coordinates
        const mars_x = Math.floor(x + 100);
        const mars_y = Math.floor(600 - y + 100);
        const waypoint_data = `${mars_x},${mars_y},M`;

        // send the waypoint data to the rover over TCP
        if (rover_connected) {
            rover_socket.write(waypoint_data);
            console.log(`Waypoint sent`);
        } else {
            console.log(`Can't send waypoint, rover is not connected`);
        }

        // send ack to webpage to render
        const packet_json = {
            "time": 1002,
            "type": "waypoint",
            "data": {
                "posX": x,
                "posY": y, 
            }
        }
        io.emit('update', packet_json);
    });

    sock.on('auto', (data) => {
        print(`Rover set to Automatic Mode`, 0);
        const tcp_send = `0,0,A`;
        // send the waypoint data to the rover over TCP
        if (rover_connected) {
            rover_socket.write(tcp_send);
            console.log(`Command sent`);
        } else {
            console.log(`Can't send, rover is not connected`);
        }
    });

    sock.on('start_mission', (msg) => {
        print(`Mission Start`, 0);
    });

    sock.on('test', ({ x, y }) => {
        print(`test point set at x: ${x}, y: ${y}`, 0);
        const time_string = (new Date()).toISOString();
        // const waypoint_data = JSON.stringify({time: time_string, x: 100, y: 120, mode: 'M'});
        const waypoint_data = `${x + 100},${y + 100},M`;
        // send the waypoint data to the rover over TCP
        if (rover_connected) {
            rover_socket.write(waypoint_data);
        } else {
            console.log(`Can't send waypoint, rover is not connected`);
        }
    });
    
});

//--------Initialising TCP server for Rover--------//
const Net = require('net');
const port = 8080;

const rover_server = new Net.Server();
var rover_connected = false;
var rover_socket;

rover_server.listen(port, () => {
    console.log(`Rover server listening on port ${port}`);
});

rover_server.on('connection', (socket) => {
    // set the global variable
    rover_socket = socket;
    rover_connected = true;
    console.log('Rover Connected');

    socket.on('data', function(chunk) {
        console.log(`Rover Data:`);
        const rover_string = chunk.toString();
        console.log(rover_string);
    });

    // socket.on('data', function(chunk) {
    //     console.log(`Rover Data:`);
    //     const rover_string = chunk.toString();
    //     const packet_list = rover_string.split('@');
    //     console.log(packet_list.length);
    //     let packet_json;
    //     packet_list.forEach(item => {
    //         if (item != '') {
    //             try {
    //                 packet_json = JSON.parse(item);
    //                 //console.log(packet_json);
    //                 //io.emit('update', packet_json);
    //             } catch (error) {
    //                 console.log(rover_string, "(not JSON)");
    //             }
    //         }
    //     });
    //     console.log("finish");
    // });

    // When the client requests to end the TCP connection with the server, the server
    // ends the connection.
    socket.on('end', function() {
        console.log('Closing connection with rover');
    });

    socket.on('error', function(err) {
        console.log(`Error: ${err}`);
    });      
});

// shutdown server gracefully
const shutdown = async () => {
    console.log('Closing HTTP server');
    await io.close();
    console.log('Closing TCP server');
    await rover_server.close();
    process.exit(0);
};

process.on('SIGINT', () => {
    console.log('SIGINT signal received');
    shutdown();
});

process.on('SIGTERM', () => {
    console.log('SIGTERM signal received');
    shutdown();
});


// var state = {
//     "rover": {
//         "posX": 100,
//         "posY": 100,
//         "angle": 0  
//     },
//     "alien": [
//         {
//             "colour": "red",
//             "posX": 50,
//             "posY": 70
//         },
//         {
//             "colour": "blue",
//             "posX": 150,
//             "posY": 70
//         },
//         {
//             "colour": "green",
//             "posX": 250,
//             "posY": 70
//         } 
//     ],
//     "building": [
//         {
//             "posX": 50,
//             "posY": 170
//         },
//         {
//             "posX": 150,
//             "posY": 170
//         },
//         {
//             "posX": 250,
//             "posY": 170
//         } 
//     ]
// }

// var count = 0;

// var interval
// interval = setInterval(() => {
//         console.log(count);
//         count++;
//     }, 500);

//--------Initialising Python Control Script--------//
// const {spawn} = require('child_process');
// const control_script = spawn('python', ['main.py']);
// print("Control Script started", 0)

// control_script.stdout.on('data', (data) => {
//     print(`${data}`, 1);
// })
// control_script.on('close', (code) => {
//     print(`Control Script ended with code ${code}`, 0)
// })