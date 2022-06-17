canvas = document.getElementById('mapCanvas');
ctx = canvas.getContext("2d");

const canvas_height = 600;
const canvas_width = 600;

function toRadians(degrees) {
    return Math.PI / 180 * degrees;
}

var rover = new Image();
var flag = new Image();

const alienRadius = 10;
const buildingRadius = 20

rover.src = "tank.png";   // load image
flag.src = "flag.png";

function initRover(){
    drawRover({"posX": 0, "posY": 0, "angle": 90});
}

function arena_to_map(_x, _y) {
    return {x: _x, y: 600 - _y};
}

rover.onload = initRover;

var state;

function drawRover(roverEntity) {
    const {x, y} = arena_to_map(roverEntity.posX, roverEntity.posY);
    ctx.translate( x + rover.width/2, y + rover.height/2);
    ctx.rotate(roverEntity.angle * Math.PI / 180);
    ctx.drawImage(rover, -rover.width/2, -rover.width/2, rover.width/2, rover.width/2);
    ctx.rotate(-1 * roverEntity.angle * Math.PI / 180);        
    ctx.translate( -1 * x - rover.width/2, -1 * y - rover.height/2);
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

function toRadians(degrees) {
    return Math.PI / 180 * degrees;
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

function redrawCanvas(entity) {
    console.log("redraw");
    //var entityData = JSON.parse(entity);
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    drawRover(entity.rover);

    for (let a in entity.alien){
        drawAlien(entity.alien[a], a);
    }
    // for (let i = 0; i < entity.alien.length; i++){
    //     drawAlien(entity.alien[i]);
    // }
    for (let i = 0; i < entity.building.length; i++){
        drawBuilding(entity.building[i]);
    }
    //requestAnimationFrame(updatePosition)
}

const getClickCoordinates = (element, ev) => {
    const { top, left } = element.getBoundingClientRect();
    const { clientX, clientY } = ev;
    return {
        x: clientX - left,
        y: clientY - top
    };
};

(() => {
    const sock = io();

    const onClick = (e) => {
        console.log("onClick");
        const { x, y } = getClickCoordinates(canvas, e);
        sock.emit('waypoint', { x, y });

    };
    canvas.addEventListener('click', onClick)

    sock.on('update', (data) => {
        redrawCanvas(data);
        state = data;
    });

    sock.on('waypoint', ({x, y}) => {
        console.log("waypoint");
        redrawCanvas(state);
        ctx.drawImage(flag, x, y - flag.height);
    });

    const start_btn = document.getElementById("start");
    start_btn.addEventListener('click', (event) => {
        console.log("Start Mission");
        sock.emit('start_mission', "");
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

})();

const _rover = {
    "time": 1002,
    "type": "rover",
    "data": {
        "posX": 100,
        "posY": 500,
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

state = {
    "rover": {
        "posX": 200,
        "posY": 200,
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
    ]
}