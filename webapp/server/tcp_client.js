// Include Nodejs' net module.
const Net = require('net');
// The port number and hostname of the server.
const port = 8080;
const host = 'localhost';

// Create a new TCP client.
const client = new Net.Socket();
// Send a connection request to the server.
client.connect({ port: port, host: host }, function() {
    // If there is no error, the server has accepted the request and created a new 
    // socket dedicated to us.
    console.log('TCP connection established with the server.');

    // The client can now send data to the server by writing to its socket.
    
    client.write('Hello, server.');
});

// // The client can also receive data from the server by reading from its socket.
// client.on('data', function(chunk) {
//     console.log(`Data received from the server: ${chunk.toString()}.`);
    
//     // Request an end to the connection after the data has been received.
//     // client.end();
// });

client.on('end', function() {
    console.log('Requested an end to the TCP connection');
});

//--------Initialising TCP server for Rover--------//
const rover_port = 8090;

const rover_server = new Net.Server();
var rover_connected = false;
var rover_socket;

rover_server.listen(rover_port, () => {
    console.log(`Rover server listening on port ${rover_port}`);
});

rover_server.on('connection', (socket) => {
    console.log('Rover Connected');

    socket.on('data', function(chunk) {
        console.log(`Rover Data:`);
        const rover_string = chunk.toString();
        client.write(rover_string);
        console.log(rover_string);
    });

    // When the client requests to end the TCP connection with the server, the server
    // ends the connection.
    socket.on('end', function() {
        console.log('Closing connection with rover');
    });

    socket.on('error', function(err) {
        console.log(`Error: ${err}`);
    });      
});