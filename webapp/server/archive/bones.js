//--------Initialising Server--------//
const http_port = 3000;
const http = require('http');

const express = require('express');
const app = express();

const server = http.createServer(app);
app.use(express.static(`${__dirname}/../client`));

server.on('error', (err) => {
    console.error(err)
});

server.listen(http_port,'localhost', () => {
    print(`Server is running on port ${http_port}`, 0)
});
