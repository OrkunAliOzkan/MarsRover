const getMap = (canvas) => {
    const ctx = canvas.getContext('2d')

    const renderBackground = () => {
        ctx.fillStyle = "#c1440e";
        ctx.fillRect(0, 0, 1200, 1200);
    };

    const makeSquare = (x, y, color) => {
        ctx.fillStyle = color;
        ctx.fillRect(x, y, 20, 20);
    };
    
    return { renderBackground, makeSquare };
};

(() => {
    const canvas = document.querySelector("#map");
    const ctx = canvas.getContext('2d');

    const {renderBackground, makeSquare} = getMap(canvas);

    renderBackground();
    makeSquare(100, 100, "green");

    const sock = io();
})();