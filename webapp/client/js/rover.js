//"use strict";

// var key_states = {
//     "w": false,
//     "a": false,
//     "s": false,
//     "d": false
// }

// const keyHandler = e => {
//     console.log(e.key);
//     document.removeEventListener(document, keyHandler)
// }

// document.addEventListener('keydown', keyHandler);

// document.addEventListener('keyup', event => {
//     document.addEventListener('keydown', event => {
//         console.log(e.key);
//     }, {once: true});
// });

const canvas = document.getElementById('mapCanvas');
const ctx = canvas.getContext("2d");

const canvas_height = 600;
const canvas_width = 913;

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

const rover_width = 50;
const rover_height = 40;

var rover = new Image(rover_width, rover_height);
var flag = new Image();

const alienRadius = 10;
const buildingRadius = 20

rover.src = "../images/rover_3.png";   // load image
flag.src = "../images/flag.png";

rover.onload = () => {
    redrawCanvas(state);
}
    
function arena_to_map(_x, _y) {
    return {x: _x, y: canvas_height - _y};
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
    rover_x.innerHTML = `${data.posX.toFixed(3)}`;
    rover_y.innerHTML = `${data.posY.toFixed(3)}`;
    rover_angle.innerHTML = `${(data.angle / Math.PI * 180.0).toFixed(3)}`;
}

function updateState(state, packet) {
    if (packet.type == "rover") {
        state.rover = packet.data;
        updateDashboard(packet.data);
    } else if (packet.type == "alien") {
        state.alien[packet.colour] = packet.data;
    } else if (packet.type == "building") {
        state.building.push(packet.data);
    } else if (packet.type == "waypoint") {
        state.waypoint = packet.data;
    }
    console.log("updated");
}

const getClickCoordinates = (element, ev) => {
    const { top, left } = element.getBoundingClientRect();
    const { clientX, clientY } = ev;
    return {
        x: Math.floor(clientX - left),
        y: Math.floor(clientY - top)
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

var mode_global = "auto";
var mission_global = false;

function openCity(evt, cityName) {
    console.log(cityName);
    // Declare all variables
    var i, tabcontent, tablinks, tabmap;

    // Get all elements with class="tablinks" and remove the class "active"
    tablinks = document.getElementsByClassName("tablinks");
    for (i = 0; i < tablinks.length; i++) {
      tablinks[i].className = tablinks[i].className.replace(" active", "");
    }
  
    // Show the current tab, and add an "active" class to the link that opened the tab
    document.getElementById(cityName).style.display = "block";
    // document.getElementById(cityName + "Map").style.display = "block";
    evt.currentTarget.className += " active";

    mode_global = cityName;
  }

(() => {
    const sock = io();

    const onClick = (e) => {
        const { x, y } = getClickCoordinates(canvas, e);
        console.log(x, y);
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

    sock.on('rover_connected', () => {
        document.getElementById("rover_status").innerHTML = "Online"; 
    });

    const start_btn = document.getElementById("start");
    start_btn.addEventListener('click', (event) => {
        console.log("Start Mission");
        sock.emit('start_mission', mode_global);
    });

    const end_btn = document.getElementById("end");
    end_btn.addEventListener('click', (event) => {
        console.log("End Mission");
        sock.emit('end_mission', "");
    });

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

    updateDashboard({posX: 124.1235, posY: 425.21, angle: 1.57});
})();
