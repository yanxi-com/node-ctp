#ifndef N_TRADER_H_
#define N_TRADER_H_

#include "stdafx.h"
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <node.h>
#include <node_object_wrap.h>
#include <nan.h>
#include "ThostFtdcTraderApi.h"
#include "ThostFtdcUserApiDataType.h"
#include <uv.h>
#include "uv_trader.h"

using namespace v8;

extern bool islog;
extern void logger_cout(const char* content);
extern std::string to_string(int val);
extern std::string charto_string(char val);

class WrapTrader : public node::ObjectWrap {
public:
	WrapTrader(void);
	~WrapTrader(void);

	///连接前置机
	static NAN_METHOD(Connect);
	///注册事件
	static NAN_METHOD(On);
	///用户登录请求
	static NAN_METHOD(ReqUserLogin);
	///登出请求
	static NAN_METHOD(ReqUserLogout);
	///投资者结算结果确认
	static NAN_METHOD(ReqSettlementInfoConfirm);
	///请求查询合约
	static NAN_METHOD(ReqQryInstrument);
	///请求查询资金账户
	static NAN_METHOD(ReqQryTradingAccount);
	///请求查询投资者持仓
	static NAN_METHOD(ReqQryInvestorPosition);
	///持仓明细
	static NAN_METHOD(ReqQryInvestorPositionDetail);
	///报单录入请求
	static NAN_METHOD(ReqOrderInsert);
	///报单操作请求
	static NAN_METHOD(ReqOrderAction);
	///请求查询合约保证金率
	static NAN_METHOD(ReqQryInstrumentMarginRate);
	///请求查询行情
	static NAN_METHOD(ReqQryDepthMarketData);
	///请求查询投资者结算结果
	static NAN_METHOD(ReqQrySettlementInfo);
	///删除接口对象
	static NAN_METHOD(Disposed);
	//对象初始化
	static void Init(int args);

	static NAN_METHOD(NewInstance);
	static NAN_METHOD(GetTradingDay);

private:
	static void initEventMap();
	static NAN_METHOD(New);
	static void pkg_cb_userlogin(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_userlogout(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_confirm(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_orderinsert(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_errorderinsert(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_orderaction(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_errorderaction(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rspqryorder(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rtnorder(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqtrade(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rtntrade(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqinvestorposition(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqinvestorpositiondetail(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqtradingaccount(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqinstrument(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqdepthmarketdata(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqsettlementinfo(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rsperror(CbRtnField* data, Local<Value>*cbArray);

	static Local<Value> pkg_rspinfo(void *vpRspInfo);
	uv_trader* uvTrader;
	static int s_uuid;
	static void FunCallback(CbRtnField *data);
	static void FunRtnCallback(int result, void* baton);
	static Persistent<Function> constructor;
	static std::map<const char*, int,ptrCmp> event_map;
	static std::map<int, Persistent<Function> > callback_map;
	static std::map<int, Persistent<Function> > fun_rtncb_map; 	
};



#endif