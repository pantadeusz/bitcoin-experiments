const exec = require('child_process').exec;






function generatetransaction(idx, backaddr, amountback, utxo, message) {
    //let idx=1;
    //let backaddr='tb1qs8lhy6fqznclyuuv8ct2zsn8rr9j9ukdn5m72p';
    //let amountback=0.0009;
    //let utxo='26a1a95089d1d84b7f17faef40c3c62eb0d677d8ff72367535062d0f4d31a229';
    let generate_cmnd = `./my_addr -idx ${idx} -backaddr ${backaddr} -backsats ${amountback} -utxo ${utxo} -msg ${message}`
    console.log(generate_cmnd);
    exec(generate_cmnd, (error, rawtxunsigned, stderr) => {
        console.log("DONE:", rawtxunsigned);
        exec(`bitcoin-cli sendrawtransaction ${rawtxunsigned}`, (error, signedrawtransactionstr, stderr) => {
            //            exec(`bitcoin-cli signrawtransactionwithwallet ${rawtxunsigned}`,(error,signedrawtransactionstr,stderr)=>{
            let signedrawtransaction = JSON.parse(signedrawtransactionstr);
            if (signedrawtransaction.complete) {
                console.log("signed:", signedrawtransaction.hex);
                //exec(`bitcoin-cli sendrawtransaction ${signedrawtransaction.hex}`,(error,result,stderr)=>{console.log("TXID",result);});
            } else {
                console.log("error signing transaction.", signedrawtransaction);
            }
        });
    });
}

exec('bitcoin-cli listunspent', (error, unspentlist, stderr) => {
    let utxoset = JSON.parse(unspentlist).sort((a, b) => {
        //        console.log(a,b);
        return (a.amount == b.amount) ? 0 : ((a.amount > b.amount) ? -1 : 1);
    });
    console.log(utxoset);
    let tx = utxoset[0];
    if ((tx.amount - 0.00001000) > 0) {
        if (process.argv[2]) {
            console.log(process.argv[2]);
            generatetransaction(tx.vout, tx.address, tx.amount - 0.00001000, tx.txid, process.argv[2]);
        } else {
            console.log("give me message");
        }
    } else {
        console.log("wallet is empty.. sorry");
    }
}); 
