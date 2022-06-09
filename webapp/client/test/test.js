canvas = document.getElementById('mapCanvas');
ctx = canvas.getContext("2d");

var rover = new Image();
var flag = new Image();

const alienRadius = 10;
const buildingRadius = 20

rover.src = "tank.png";   // load image
flag.src = "flag.png";

function initRover(){
    ctx.drawImage(rover, 0, canvas.height - 160);
}

rover.onload = initRover;

var state;

function drawRover(roverEntity) {
    //if(rx != roverEntity.posX || ry != roverEntity.posY) {
        ctx.translate( roverEntity.posX + rover.width/2, roverEntity.posY + rover.height/2);
        ctx.rotate(roverEntity.angle * Math.PI / 180);
        ctx.drawImage(rover, -rover.width/2, -rover.width/2);
        ctx.rotate(-1 * roverEntity.angle * Math.PI / 180);        
        ctx.translate( -1 * roverEntity.posX - rover.width/2, -1 * roverEntity.posY - rover.height/2);
    //}
}

function drawAlien(alienEntity) {
    //if(ax != alienEntity.posX || ay != alienEntity.posY) {
        ctx.beginPath();
        ctx.arc(alienEntity.posX, alienEntity.posY, alienRadius, 0, 2 * Math.PI, false);
        ctx.fillStyle = alienEntity.colour;
        ctx.fill();
        ctx.lineWidth = 5;
        ctx.strokeStyle = '#003300';
        ctx.stroke();
    //}
}

function drawBuilding(buildingEntity) {
    //if(ax != alienEntity.posX || ay != alienEntity.posY) {
        ctx.beginPath();
        ctx.arc(buildingEntity.posX, buildingEntity.posY, buildingRadius, 0, 2 * Math.PI, false);
        ctx.fillStyle = "grey";
        ctx.fill();
        ctx.lineWidth = 5;
        ctx.strokeStyle = "grey";
        ctx.stroke();
    //}
}

function redrawCanvas(entity) {
    console.log("redraw");
    //var entityData = JSON.parse(entity);
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    drawRover(entity.rover);
    for (let i = 0; i < entity.alien.length; i = i+1){
        drawAlien(entity.alien[i]);
    }
    for (let i = 0; i < entity.alien.length; i = i+1){
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
        redrawCanvas(state);
        ctx.drawImage(flag, x, y - flag.height);
    });

})();

state = {
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