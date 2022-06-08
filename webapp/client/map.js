canvas = document.getElementById('mapCanvas');
ctx = canvas.getContext("2d");

var rover = new Image(),
    x = 0,
    y = canvas.height - 160,
    angle = 0;

rover.onload = initRover;
rover.src = "tank.png";   // load image

function initRover(){
    ctx.drawImage(rover, x, y);
}

function updatePosition(entity) {
    console.log(entity);
    //ctx.clearRect(0, 0, canvas.width, canvas.height);
    if (entity.name == "rover") {
        // if(x != entity.pos[0] || y != entity.pos[1]) {
        //     x = entity.pos[0];
        //     y = entity.pos[1];
        // }
        x = (entity.pos).x;
        y = (entity.pos).y;
        ctx.drawImage(rover, x, y);
    }
    //requestAnimationFrame(updatePosition)
}

(() => {
    const sock = io();
    sock.on('rover-update', (rover_data) => updatePosition(rover_data));
    // sock.on('alien-update', (alien_data) => console.log(alien_data));
})();