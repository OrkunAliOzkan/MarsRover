var wrapperUP = document.getElementById('counterUP');
var wrapperDOWN = document.getElementById('counterDOWN');
var wrapperLEFT = document.getElementById('counterLEFT');
var wrapperRIGHT = document.getElementById('counterRIGHT');
var counterUP, counterDOWN, counterLEFT, counterRIGHT;
var countUP = 0, countDOWN = 0, countLEFT = 0, countRIGHT = 0;

var form = document.getElementById('data_input');

function onSubmit(event) {
    if (event) { event.preventDefault(); }
    console.log('submitting');
    postFormData(form);
}
form.addEventListener('submit', onSubmit, false);
// prevent submit() calls by overwriting the method
form.submit = onSubmit;

function startUP() {
    counterUP = setInterval(function() {
        wrapperUP.innerHTML = countUP;
        console.log(countUP);
        countUP++;
    }, 50);
}

function endUP() {
    clearInterval(counterUP)
}

function startLEFT() {
    counterLEFT = setInterval(function() {
        wrapperLEFT.innerHTML = countLEFT;
        console.log(countLEFT);
        countLEFT++;
    }, 50);
}

function endLEFT() {
    clearInterval(counterLEFT)
}

function startRIGHT() {
    counterRIGHT = setInterval(function() {
        wrapperRIGHT.innerHTML = countRIGHT;
        console.log(countRIGHT);
        countRIGHT++;
    }, 50);
}

function endRIGHT() {
    clearInterval(counterRIGHT)
}

function startDOWN() {
    counterDOWN = setInterval(function() {
        wrapperDOWN.innerHTML = countDOWN;
        console.log(countDOWN);
        countDOWN++;
    }, 50);
}

function endDOWN() {
    clearInterval(counterDOWN)
}