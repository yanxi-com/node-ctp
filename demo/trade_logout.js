var ctp = require('bindings')('shifctp');
var config = require('./config');

ctp.settings({log: true});

var trader = ctp.createTrader();

trader.on("connect", function (result) {
    console.log("on connected");
    trader.reqUserLogin(config.brokerID, config.investorID, config.password, function (result, iRequestID) {
        console.log('login return val is ' + result);
    });

});

trader.on('rspUserLogin', function (requestId, isLast, field, info) {

    console.log(JSON.stringify(field));
    console.log(info);
    trader.reqUserLogout(config.brokerID, config.investorID, function (result, iRequestID) {
        console.log('logout return val is ' + result);

    });
});

trader.on('rspUserLogout', function (requestId, isLast, field, info) {
    console.log('rspUserLogout');
    console.log(JSON.stringify(field));
    console.log(info);

});

trader.on('rspError', function (requestId, isLast, field) {
    console.log(JSON.stringify(field));

});

trader.connect(config.tradePoints, undefined, 0, 1, function (result) {
    console.log('connect return val is ' + result);
});

console.log('continute');
