// taken from https://www.cssscript.com/touch-joystick-controller/ 
// joystickcontroller is !!NOT OUR OWN WORK!!
class JoystickController
{
	// stickID: ID of HTML element (representing joystick) that will be dragged
	// maxDistance: maximum amount joystick can move in any direction
	// deadzone: joystick must move at least this amount from origin to register value change
	constructor( stickID, maxDistance, deadzone, socket )
	{
		this.id = stickID;
		let stick = document.getElementById(stickID);

		// location from which drag begins, used to calculate offsets
		this.dragStart = null;

		// track touch identifier in case multiple joysticks present
		this.touchId = null;
		
		this.active = false;
		this.value = { x: 0, y: 0 }; 

		let self = this;

		function handleDown(event)
		{
		    self.active = true;

			// all drag movements are instantaneous
			stick.style.transition = '0s';

			// touch event fired before mouse event; prevent redundant mouse event from firing
			event.preventDefault();

		    if (event.changedTouches)
		    	self.dragStart = { x: event.changedTouches[0].clientX, y: event.changedTouches[0].clientY };
		    else
		    	self.dragStart = { x: event.clientX, y: event.clientY };

			// if this is a touch event, keep track of which one
		    if (event.changedTouches)
		    	self.touchId = event.changedTouches[0].identifier;
		}
		
		function handleMove(event) 
		{
		    if ( !self.active ) return;

		    // if this is a touch event, make sure it is the right one
		    // also handle multiple simultaneous touchmove events
		    let touchmoveId = null;
		    if (event.changedTouches)
		    {
		    	for (let i = 0; i < event.changedTouches.length; i++)
		    	{
		    		if (self.touchId == event.changedTouches[i].identifier)
		    		{
		    			touchmoveId = i;
		    			event.clientX = event.changedTouches[i].clientX;
		    			event.clientY = event.changedTouches[i].clientY;
		    		}
		    	}

		    	if (touchmoveId == null) return;
		    }

		    const xDiff = event.clientX - self.dragStart.x;
		    const yDiff = event.clientY - self.dragStart.y;
		    let angle = Math.atan2(yDiff, xDiff);
			let distance = Math.min(maxDistance, Math.hypot(xDiff, yDiff));
			const xPosition = distance * Math.cos(angle);
			const yPosition = distance * Math.sin(angle);

			// move stick image to new position
		    stick.style.transform = `translate3d(${xPosition}px, ${yPosition}px, 0px)`;
			// angle = -1 * angle - Math.PI / 2;
			// if (angle < -1 * Math.PI) {
			// 	angle = 2 * Math.PI + angle;
			// }
			// angle = angle / Math.PI * 180;
			// distance = distance * 8;
			// console.log(`distance : ${distance}, angle: ${angle}`);
            // socket.emit("stickmove", {d: distance, a: angle});
			
			// deadzone adjustment
			const distance2 = (distance < deadzone) ? 0 : maxDistance / (maxDistance - deadzone) * (distance - deadzone);
		    const xPosition2 = distance2 * Math.cos(angle);
			const yPosition2 = distance2 * Math.sin(angle);
		    const xPercent = parseFloat((xPosition2 / maxDistance).toFixed(4));
		    const yPercent = parseFloat((yPosition2 / maxDistance).toFixed(4));
		    
		    self.value = { x: xPercent, y: yPercent };
			const x_data = xPercent * 512;
			const y_data = yPercent * -512;
			console.log(x_data, y_data);
			socket.emit("stickmove", {x: x_data, y: y_data});
		  }

		function handleUp(event) 
		{
		    if ( !self.active ) return;

		    // if this is a touch event, make sure it is the right one
		    if (event.changedTouches && self.touchId != event.changedTouches[0].identifier) return;

		    // transition the joystick position back to center
		    stick.style.transition = '.2s';
		    stick.style.transform = `translate3d(0px, 0px, 0px)`;

		    // reset everything
		    self.value = { x: 0, y: 0 };
		    self.touchId = null;
		    self.active = false;
		}

		stick.addEventListener('mousedown', handleDown);
		stick.addEventListener('touchstart', handleDown);
		document.addEventListener('mousemove', handleMove, {passive: false});
		document.addEventListener('touchmove', handleMove, {passive: false});
		document.addEventListener('mouseup', handleUp);
		document.addEventListener('touchend', handleUp);
	}
}

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
    const canvas = document.querySelector("#map");

    const {renderBackground, makeSquare} = getMap(canvas);

    renderBackground();
    makeSquare(100, 100, "blue");

    const onClick = (e) => {
        const { x, y } = getClickCoordinates(canvas, e);
        sock.emit('waypoint', { x, y });
    };

    sock.on('waypoint', ({ x, y }) => makeSquare(x-10, y-10))

    let joystick1 = new JoystickController("stick", 64, 8, sock);

    function loop()
    {
        requestAnimationFrame(loop);
    }

    loop();

    canvas.addEventListener('click', onClick)
    
})();

