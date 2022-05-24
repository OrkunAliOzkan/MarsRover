console.log("Hello World");

let person = {
    name : "Bryan",
    email : "bet20@ic.ac.uk",
    numMods : 8,
    intro : function (input)
    {
        return input + this.name + "\n" + this.email;
    }
};

console.log(person.intro("hi"));

console.log(person.name + " " + person.email)

let num1=4, num2=5;
console.log("Sum: ", num1+num2);

let num = 5, username = "s.baig", domain = "@imperial.ac.uk", type = true;
console.log(username+domain);



// functional programming
function getArea(shape)
{
    if (shape == "circle") {
        return function(r){ return Math.PI * r * r;}
    } else if (shape == "rectangle") {
        return function(a, b){ return a * b; }
    }
}

c_area = getArea("rectangle");
console.log(c_area(12,11));

function greeting(obj){
    obj.logMessage = function(){
        console.log(this.name, "is", this.age, "years old!");
    }
};

const tom = {
    name: "Tom",
    age: 420
};

const jerry = {
    name: "jerry",
    age: 69
};

greeting(tom);
greeting(jerry);