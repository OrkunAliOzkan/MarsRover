const arena_height = 2333;
const arena_width = 3555;

function calc_quadrant(posX, posY) {
    return Math.floor(posY / (arena_height / 2)) + Math.floor(posX / (arena_width / 3)) * 2;
}

console.log(calc_quadrant(1000, 2000));