let intro = document.querySelector(".intro");
let logo = document.querySelector(".logo-header");
let logoSpan = document.querySelectorAll(".logo");   //Multiple elements

let box = document.querySelectorAll(".boxes");

//Eventlistener added to full window
//Triggers once all DOM contenet on page fully loaded.
window.addEventListener("DOMContentLoaded", () => {
    /* INTRO*/
    setTimeout(() => {
        //Select logoSpan elements. Animations applied to both so forEach loop used.
        //Function takes in each individual span element and index number (0 and 1).
        logoSpan.forEach((span, index) => {
            setTimeout(() => {
                //Add the active class
                span.classList.add("active");
                // take index number and add 1 then * by 400 ms. 1 is added so the delay is not 0.
            }, (index + 1) * 400)
        });
    });

    setTimeout(() => {
        logoSpan.forEach((span, index) => {
            setTimeout(() => {
                //Add the fade class and remove the active calss
                span.classList.remove("active");
                span.classList.add("fade");
                //span.classList.add("fade");
                // take index number and add 1 then * by 50 ms. 1 is added so the delay is not 0.
            }, (index + 1) * 50)
        });
    }, 2000);

    setTimeout(() => {
        intro.style.top = "-100vh";
    }, 2300)


    /* BOXES APPEAR */
    setTimeout(() => {
        //Select logoSpan elements. Animations applied to both so forEach loop used.
        //Function takes in each individual span element and index number (0 and 1).
        box.forEach((span, index) => {
            setTimeout(() => {
                //Add the active class
                span.classList.add("active");
                // take index number and add 1 then * by 400 ms. 1 is added so the delay is not 0.
            }, (index + 1) * 400)
        });
    }, 2700);


})