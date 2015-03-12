var ctp = require('bindings')('shifctp');
var config = require('./config');

ctp.settings({log: true});

var trader = ctp.createTrader();

trader.on("connect", function (result) {
    console.log("on connected");
    trader.reqUserLogin(config.brokerID, config.investorID, config.password, function (result) {
        console.log('login return val is ' + result);
    });

});

trader.on('rspUserLogin', function (requestId, isLast, field, info) {

    console.log(JSON.stringify(field));
    console.log(info);

    trader.reqQryDepthMarketData('ag1506', function (result) {
        console.log('rqDdpthmarketData return val is ' + result);

    });
});

trader.on('rqDdpthmarketData', function (requestId, isLast, field, info) {
    console.log('rqDdpthmarketData callback');
    console.log(field);
    console.log(info);

});

trader.on('rtnOrder', function (field) {
    console.log(field);
});

trader.on('rspError', function (requestId, isLast, field) {
    console.log(JSON.stringify(field));

});

trader.connect(config.tradePoints, undefined, 0, 1, function (result) {
    console.log('connect return val is ' + result);
});

console.log('continute');
