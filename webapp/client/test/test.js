canvas = document.getElementById('mapCanvas');
ctx = canvas.getContext("2d");

const canvas_height = 600;
const canvas_width = 600;

function toRadians(degrees) {
    return Math.PI / 180 * degrees;
}

var state;

state = {
    "rover": {
        "posX": 60, 
        "posY": 60, 
        "angle": 0
    },
    "alien": {
        "red": {
            "posX": 100,
            "posY": 500
        },
        "blue": {
            "posX": 200,
            "posY": 500
        },
        "green": {
            "posX": 300,
            "posY": 500
        }
    },
    "building": [
        {
            "posX": 50,
            "posY": 300
        },
        {
            "posX": 150,
            "posY": 300
        },
        {
            "posX": 250,
            "posY": 300
        } 
    ],
    "fan": {
        "posX": 50,
        "posY": 300
    }
}

const _rover = {
    "time": 1002,
    "type": "rover",
    "data": {
        "posX": 400,
        "posY": 469,
        "angle": 0  
    }
}

const _alien = {
    "time": 1002,
    "type": "alien",
    "colour": "blue",
    "data": {
        "posX": 150,
        "posY": 70,
    }
}

rover_width = 50;
rover_height = 40;

var rover = new Image(rover_width, rover_height);
var flag = new Image();

const alienRadius = 10;
const buildingRadius = 20

rover.src = "rover_3.png";   // load image
flag.src = "flag.png";

rover.onload = () => {
    redrawCanvas(state);
}
    
function arena_to_map(_x, _y) {
    return {x: _x, y: 600 - _y};
}


function drawPointer(x, y) {
    ctx.beginPath();
    ctx.arc(x, y, 3, 0, 2 * Math.PI, false);
    ctx.fillStyle = "red";
    ctx.fill();
    ctx.stroke();
}

function drawRover(roverEntity) {
    const {x, y} = arena_to_map(roverEntity.posX, roverEntity.posY);
    const img_x = x - rover.width/2;
    const img_y = y - rover.height/2;
    const rover_angle = (roverEntity.angle * -1 + Math.PI / 2);

    // move and rotate
    ctx.translate(x, y);
    ctx.rotate(rover_angle);
    ctx.translate(-x, -y);       

    // draw
    ctx.drawImage(rover, img_x, img_y, rover.width, rover.height);

    // reverse
    ctx.translate(x, y);
    ctx.rotate(-rover_angle);
    ctx.translate(-x, -y);  
}

function drawAlien(alienEntity, colour) {
    const {x, y} = arena_to_map(alienEntity.posX, alienEntity.posY);
    ctx.beginPath();
    ctx.arc(x, y, alienRadius, 0, 2 * Math.PI, false);
    ctx.fillStyle = colour;
    ctx.fill();
    // ctx.lineWidth = 5;
    // ctx.strokeStyle = '#003300';
    ctx.stroke();
}

function drawBuilding(buildingEntity) {
    const {x, y} = arena_to_map(buildingEntity.posX, buildingEntity.posY);
    for (let i = 0; i < 12; i++){
        ctx.fillStyle = i % 2 == 0 ? "black": "white";
        ctx.beginPath();
        ctx.moveTo(x,y);
        ctx.arc(x, y, buildingRadius, i * 2 * Math.PI / 12, (i+1) * 2 * Math.PI / 12, false);
        ctx.lineTo(x,y);
        ctx.closePath();
        ctx.fill();
    }
    ctx.fillStyle = "grey";
    ctx.beginPath();
    ctx.moveTo(x,y);
    ctx.arc(x, y, buildingRadius - 5, 0, 360, false);
    ctx.closePath();
    ctx.fill();
}

function drawWaypoint(waypointEntity) {
    ctx.drawImage(flag, waypointEntity.posX, waypointEntity.posY - flag.height);
}

function redrawCanvas(entity) {
    console.log("redraw");
    //var entityData = JSON.parse(entity);
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // draw aliens
    for (let a in entity.alien){
        drawAlien(entity.alien[a], a);
    }

    // draw buildings
    for (let i = 0; i < entity.building.length; i++){
        drawBuilding(entity.building[i]);
    }

    // draw rover
    drawRover(entity.rover);

    // draw waypoint
    if (entity.waypoint != null) {
        drawWaypoint(entity.waypoint);
    }
}

const rover_x = document.getElementById("rover_x");
const rover_y = document.getElementById("rover_y");
const rover_angle = document.getElementById("rover_angle");

function updateDashboard(data) {
    rover_x.innerHTML = `Rover x: ${data.posX}`;
    rover_y.innerHTML = `Rover y: ${data.posY}`;
    rover_angle.innerHTML = `Rover angle: ${data.angle / Math.PI * 180.0}`;
}

function updateState(state, packet) {
    if (packet.type == "rover") {
        state.rover = packet.data;
        updateDashboard(packet.data);
    } else if (packet.type == "alien") {
        state.alien[packet.colour] = packet.data;
    } else if (packet.type == "building") {
    } else if (packet.type == "waypoint") {
        state.waypoint = packet.data;
    }
    console.log("updated");
}

const getClickCoordinates = (element, ev) => {
    const { top, left } = element.getBoundingClientRect();
    const { clientX, clientY } = ev;
    return {
        x: clientX - left,
        y: clientY - top
    };
};

function genPacketsSmall() {
    // generate small 60 packets
    const duration = 3000;
    const packet_period = 50;
    let packet_list = [];

    const num_packets = Math.floor(duration / packet_period)
    for (let i = 0; i < num_packets; i++){
        let tmp = {
            "time": i,
            "type": "rover",
            "data": {
                "posX": 50 + 450 / num_packets * i,
                "posY": 50 + 450 / num_packets * i,
                "angle": i * 2 * Math.PI / num_packets  
            }
        }
        let packet_json = JSON.stringify(tmp);
        packet_list.push(packet_json);
    }

    packet_list.forEach(item => console.log(item));
    return packet_list;
}

// function replay_driver (intervalID, state, replay_data, curr_frame) {
//     if (curr_frame < replay_data.length) {

//     } else {

//     }
// }

(() => {
    const sock = io();

    const onClick = (e) => {
        console.log("onClick");
        const { x, y } = getClickCoordinates(canvas, e);
        sock.emit('waypoint', { x, y });
    };
    canvas.addEventListener('click', onClick)

    sock.on('update', (packet) => {
        console.log(packet);
        updateState(state, packet);
        redrawCanvas(state);
    });

    sock.on('waypoint', ({x, y}) => {
        console.log("waypoint");
    });

    const start_btn = document.getElementById("start");
    start_btn.addEventListener('click', (event) => {
        console.log("Start Mission");
        sock.emit('start_mission', "");
    })

    const auto_btn = document.getElementById("auto");
    auto_btn.addEventListener('click', (event) => {
        console.log("Auto Mode");
        sock.emit('auto', "");
    })

    const data_btn = document.getElementById("data_btn");
    data_btn.addEventListener('click', (event) => {
        const angle = Math.floor(document.getElementById("angle").value) / 180 * Math.PI;
        const distance = Math.floor(document.getElementById("distance").value);
        const x = distance * Math.cos(angle);
        const y = distance * Math.sin(angle);
        console.log(`x: ${x}, y: ${y}`);
        sock.emit('test', {x, y});
    })

    const c_btn = document.getElementById("c_btn");
    c_btn.addEventListener('click', (event) => {
        const x = Math.floor(document.getElementById("x").value);
        const y = Math.floor(document.getElementById("y").value);
        console.log(`x: ${x}, y: ${y}`);
        sock.emit('test', {x, y});
    })

    const replay_btn = document.getElementById("replay");
    replay_btn.addEventListener('click', (event) => {
        const replay_text = document.getElementById("title");
        replay_text.innerHTML = "Testing - Replay begin";
        console.log("Replay begin");
    })

    // updateState(state, _alien);

    // const replay_data_string = genPacketsSmall();
    // let replay_data_json = [];
    // for (let i = 0; i < replay_data_string.length; i++){
    //     replay_data_json.push(JSON.parse(replay_data_string[i]));
    // }
    // // replay_data_json.forEach(item => console.log(item));

    // var curr_replay_frame = 0;

    // var replay_interval;
    // replay_interval = setInterval(() => {
    //     if (curr_replay_frame < replay_data_json.length) {
    //         updateState(state, replay_data_json[curr_replay_frame]);
    //         redrawCanvas(state);
    //         curr_replay_frame++;
    //     } else {
    //         clearInterval(replay_interval);
    //     }
    // }, 50);
})();

