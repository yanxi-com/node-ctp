var ctp = require('bindings')('shifctp');

var config = require('./config');

ctp.settings({log: true});

var mduser = ctp.createMdUser();

mduser.on("connect", function (result) {
    console.log("on connected");
    mduser.reqUserLogin('', '', '', function (result) {
        console.log('reqUserLogin callback');
        console.log(result);
    });

});

mduser.on("rspUserLogin", function (requestId, isLast, field, info) {

    mduser.subscribeMarketData(['ag1506'], function (result) {
        console.log('subscribeMarketData result:' + result);
    });
});

mduser.on('rspSubMarketData', function (requestId, isLast, field, info) {

    console.log("rspSubMarketData");
});

mduser.on('rspUnSubMarketData', function (requestId, isLast, field, info) {

    mduser.disconnect();

});

mduser.on('rtnDepthMarketData', function (field) {
    console.log(JSON.stringify(field, null, 2));

});

mduser.on('rspError', function (requestId, isLast, info) {

    console.log("repError");

});


mduser.connect(config.mdPoints, undefined, function (f) {
    console.log(f);
});
