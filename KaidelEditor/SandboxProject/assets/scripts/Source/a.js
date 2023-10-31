function bin(binary){
    let sign = (-1) ** Number(binary[0]);
    let exp = 0;
    for(let i =1 ;i<=8;++i){
        exp = exp<<1;
        exp +=Number(binary[i]);
    }
    let mantissa = 1.0;
    for(let i  = 9;i<=31;++i){

    }

}