function primeOrNot(){
    console.log("hello");
    let num = Math.floor(document.getElementById("num1").value);

    function primeTest(num){
        if(num<2){
            return 1; 
        } else if (num==2){
            return -1;
        } else {
            const rootnum = Math.sqrt(num);
            for(let d=2; d<rootnum; d++){
                if(num%d==0){
                    return d;
                }
            }
            return -1;
        }
    }

    const tmp = primeTest(num);

    let p_or_not;
    let factor_info = "";
    if (tmp === -1) {
        p_or_not = " a prime ";
        factor_info = "";
    } else {
        p_or_not = " not a prime ";
        factor_info = tmp + " is a factor"
    }

    const responseContent = "The number is"+ p_or_not;
    document.getElementById("response").innerHTML=responseContent;
    document.getElementById("factor").innerHTML=factor_info;
}

