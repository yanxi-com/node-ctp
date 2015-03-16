#include <node.h>
#include "wrap_mduser.h"

Persistent<Function> WrapMdUser::constructor;
int WrapMdUser::s_uuid;
std::map<const char*, int,ptrCmp> WrapMdUser::event_map;
std::map<int, Persistent<Function> > WrapMdUser::callback_map;
std::map<int, Persistent<Function> > WrapMdUser::fun_rtncb_map;

WrapMdUser::WrapMdUser() {
	logger_cout("wrap_mduser------>object start init");
	uvMdUser = new uv_mduser();
	logger_cout("wrap_mduser------>object init successed");
}

WrapMdUser::~WrapMdUser() {
    if(uvMdUser){
	    delete uvMdUser;
    }
	logger_cout("wrape_mduser------>object destroyed");
}

void WrapMdUser::Init(int args) {
	// Prepare constructor template
	Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);

	tpl->SetClassName(NanNew("WrapMdUser"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Prototype
	NanSetPrototypeTemplate(tpl, "on", NanNew<FunctionTemplate>(On));
	NanSetPrototypeTemplate(tpl, "connect", NanNew<FunctionTemplate>(Connect));
	NanSetPrototypeTemplate(tpl, "reqUserLogin", NanNew<FunctionTemplate>(ReqUserLogin));
	NanSetPrototypeTemplate(tpl, "reqUserLogout", NanNew<FunctionTemplate>(ReqUserLogout));
	NanSetPrototypeTemplate(tpl, "subscribeMarketData", NanNew<FunctionTemplate>(SubscribeMarketData));
	NanSetPrototypeTemplate(tpl, "unSubscribeMarketData", NanNew<FunctionTemplate>(UnSubscribeMarketData));
	NanSetPrototypeTemplate(tpl, "disconnect", NanNew<FunctionTemplate>(Disposed));

	NanAssignPersistent<Function>(constructor, tpl->GetFunction());
}

void WrapMdUser::initEventMap() {
	event_map["connect"] = T_ON_CONNECT;
	event_map["disconnected"] = T_ON_DISCONNECTED;
	event_map["rspUserLogin"] = T_ON_RSPUSERLOGIN;
	event_map["rspUserLogout"] = T_ON_RSPUSERLOGOUT;
	event_map["rspSubMarketData"] = T_ON_RSPSUBMARKETDATA;
	event_map["rspUnSubMarketData"] = T_ON_RSPUNSUBMARKETDATA;
	event_map["rtnDepthMarketData"] = T_ON_RTNDEPTHMARKETDATA;
	event_map["rspError"] = T_ON_RSPERROR;
}

NAN_METHOD(WrapMdUser::New) {
    NanScope();

	if (event_map.size() == 0)
		initEventMap();
	WrapMdUser* obj = new WrapMdUser();
	obj->Wrap(args.This());
	NanReturnValue(args.This());
}

NAN_METHOD(WrapMdUser::NewInstance) {
    NanScope();

	const unsigned argc = 1;
	Handle<Value> argv[argc] = { args[0] };

	Local<Object> instance = NanNew(constructor)->NewInstance(argc, argv);
	NanReturnValue(instance);
}

NAN_METHOD(WrapMdUser::On) {
    NanScope();
	if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
		logger_cout("Wrong arguments->event name or function");
		NanThrowError(Exception::TypeError(NanNew<String>("Wrong arguments->event name or function")));
		NanReturnValue(NanUndefined());
	}

	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());

	Local<String> eventName = args[0]->ToString();
	Local<Function> cb = Local<Function>::Cast(args[1]);
	Persistent<Function> unRecoveryCb;
	NanAssignPersistent(unRecoveryCb, cb);
	NanUtf8String eNameAscii(eventName);

	std::map<const char*, int>::iterator eIt = event_map.find(*eNameAscii);
	if (eIt == event_map.end()) {
		NanThrowError(Exception::TypeError(NanNew<String>("System has no register this event")));
		NanReturnValue(NanUndefined());
	}
	std::map<int, Persistent<Function> >::iterator cIt = callback_map.find(eIt->second);
	if (cIt != callback_map.end()) {
		logger_cout("Callback is defined before");
		NanThrowError(Exception::TypeError(NanNew<String>("Callback is defined before")));
		NanReturnValue(NanUndefined());
	}

	callback_map[eIt->second] = unRecoveryCb;
//	NanAssignPersistent(callback_map[eIt->second], unRecoveryCb);

	obj->uvMdUser->On(*eNameAscii, eIt->second, FunCallback);
	NanReturnValue(NanNew<Integer>(0));
}

NAN_METHOD(WrapMdUser::Connect) {
    NanScope();
	std::string log = "wrap_mduser Connect------>";
	if (args[0]->IsUndefined()) {
		logger_cout("Wrong arguments->front addr");
		NanThrowError(Exception::TypeError(NanNew<String>("Wrong arguments->front addr")));
		NanReturnValue(NanUndefined());
	}	
	int uuid = -1;
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
	if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
		uuid = ++s_uuid;
		NanAssignPersistent(fun_rtncb_map[uuid], Local<Function>::Cast(args[2]));

		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<String> frontAddr = args[0]->ToString();
	Local<String> szPath = args[1]->IsUndefined() ? NanNew<String>("m") : args[0]->ToString();
	NanUtf8String addrAscii(frontAddr);
	NanUtf8String pathAscii(szPath);

	UVConnectField pConnectField;
	memset(&pConnectField, 0, sizeof(pConnectField));
	strcpy(pConnectField.front_addr, ((std::string)*addrAscii).c_str());
	strcpy(pConnectField.szPath, ((std::string)*pathAscii).c_str());  
	logger_cout(log.append(" ").append((std::string)*addrAscii).append("|").append((std::string)*pathAscii).append("|").c_str());
	obj->uvMdUser->Connect(&pConnectField, FunRtnCallback, uuid);
	NanReturnValue(NanUndefined());
}

NAN_METHOD(WrapMdUser::ReqUserLogin) {
    NanScope();
	std::string log = "wrap_mduser ReqUserLogin------>";
	if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		NanThrowError(Exception::TypeError(NanNew<String>("Wrong arguments")));
		NanReturnValue(NanUndefined());
	}

	int uuid = -1;
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
	if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
		uuid = ++s_uuid;
		NanAssignPersistent(fun_rtncb_map[uuid], Local<Function>::Cast(args[3]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<String> broker = args[0]->ToString();
	Local<String> userId = args[1]->ToString();
	Local<String> pwd = args[2]->ToString();
	NanUtf8String brokerAscii(broker);
	NanUtf8String userIdAscii(userId);
	NanUtf8String pwdAscii(pwd);

	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.UserID, ((std::string)*userIdAscii).c_str());
	strcpy(req.Password, ((std::string)*pwdAscii).c_str());
	logger_cout(log.append(" ").append((std::string)*brokerAscii).append("|").append((std::string)*userIdAscii).append("|").append((std::string)*pwdAscii).c_str());
	obj->uvMdUser->ReqUserLogin(&req, FunRtnCallback, uuid);
	NanReturnValue(NanUndefined());
}

NAN_METHOD(WrapMdUser::ReqUserLogout) {
    NanScope();
	std::string log = "wrap_mduser ReqUserLogout------>";

	if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		NanThrowError(Exception::TypeError(NanNew<String>("Wrong arguments")));
		NanReturnValue(NanUndefined());
	}
	int uuid = -1;
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
	if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
		uuid = ++s_uuid;
		NanAssignPersistent(fun_rtncb_map[uuid], Local<Function>::Cast(args[2]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<String> broker = args[0]->ToString();
	Local<String> userId = args[1]->ToString();
	NanUtf8String brokerAscii(broker);
	NanUtf8String userIdAscii(userId);

	CThostFtdcUserLogoutField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.UserID, ((std::string)*userIdAscii).c_str());
	logger_cout(log.append(" ").append((std::string)*brokerAscii).append("|").append((std::string)*userIdAscii).c_str());
	obj->uvMdUser->ReqUserLogout(&req, FunRtnCallback, uuid);
	NanReturnValue(NanUndefined());
}

NAN_METHOD(WrapMdUser::SubscribeMarketData) {
    NanScope();
	std::string log = "wrap_mduser SubscribeMarketData------>";

	if (args[0]->IsUndefined() || !args[0]->IsArray()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		NanThrowError(Exception::TypeError(NanNew<String>("Wrong arguments")));
		NanReturnValue(NanUndefined());
	}
	int uuid = -1;
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++s_uuid;
		NanAssignPersistent(fun_rtncb_map[uuid], Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	} 
	Local<v8::Array> instrumentIDs = Local<v8::Array>::Cast(args[0]);
	char** idArray = new char*[instrumentIDs->Length()];
	
	for (uint32_t i = 0; i < instrumentIDs->Length(); i++) {
		Local<String> instrumentId = instrumentIDs->Get(i)->ToString();
		NanUtf8String idAscii(instrumentId);
		char* id = new char[instrumentId->Length() + 1];
		strcpy(id, *idAscii);
		idArray[i] = id;
		log.append(*idAscii).append("|");
	}
	logger_cout(log.c_str());
	obj->uvMdUser->SubscribeMarketData(idArray, instrumentIDs->Length(), FunRtnCallback, uuid);
	delete idArray;
	NanReturnValue(NanUndefined());
}

NAN_METHOD(WrapMdUser::UnSubscribeMarketData) {
    NanScope();
	std::string log = "wrap_mduser UnSubscribeMarketData------>";

	if (args[0]->IsUndefined() || !args[0]->IsArray()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		NanThrowError(Exception::TypeError(NanNew<String>("Wrong arguments")));
		NanReturnValue(NanUndefined());
	}
	int uuid = -1;
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++s_uuid;
		NanAssignPersistent(fun_rtncb_map[uuid], Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}
	Local<v8::Array> instrumentIDs = Local<v8::Array>::Cast(args[0]);
	char** idArray = new char*[instrumentIDs->Length()];

	for (uint32_t i = 0; i < instrumentIDs->Length(); i++) {
		Local<String> instrumentId = instrumentIDs->Get(i)->ToString();
		NanUtf8String idAscii(instrumentId);
		char* id = new char[instrumentId->Length() + 1];
		strcpy(id, *idAscii);
		idArray[i] = id;
		log.append(*idAscii).append("|");
	}
	logger_cout(log.c_str());
	obj->uvMdUser->UnSubscribeMarketData(idArray, instrumentIDs->Length(), FunRtnCallback, uuid);
	NanReturnValue(NanUndefined());
}

NAN_METHOD(WrapMdUser::Disposed) {
    NanScope();
	WrapMdUser* obj = ObjectWrap::Unwrap<WrapMdUser>(args.This());
	obj->uvMdUser->Disposed();
	std::map<int, Persistent<Function> >::iterator callback_it = callback_map.begin();
	while (callback_it != callback_map.end()) {
		NanDisposePersistent(callback_it->second);
		callback_it++;
	}
	event_map.clear();
	callback_map.clear();
	fun_rtncb_map.clear();
	delete obj->uvMdUser;
    obj->uvMdUser = NULL;
	logger_cout("wrap_mduser Disposed------>wrap disposed");
	NanReturnValue(NanUndefined());
}


////////////////////////////////////////////////////////////////////////////////////////////////

void WrapMdUser::FunCallback(CbRtnField *data) {
	NanEscapableScope();
	std::map<int, Persistent<Function> >::iterator cIt = callback_map.find(data->eFlag);
	if (cIt == callback_map.end())
		return;

	switch (data->eFlag) {
	case T_ON_CONNECT:
	{
						 Local<Value> argv[1] = { NanUndefined() };
//						 cIt->second->Call(NanGetCurrentContext(), 1, argv);
//						 cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);
						 NanMakeCallback(NanGetCurrentContext()->Global(), NanNew(cIt->second), 1, argv);
						 break;
	}
	case T_ON_DISCONNECTED:
	{
							  Local<Value> argv[1] = { NanNew<Integer>(data->nReason) };
//							  cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);
							  NanMakeCallback(NanGetCurrentContext()->Global(), NanNew(cIt->second), 1, argv);
							  break;
	}
	case T_ON_RSPUSERLOGIN:
	{
							  Local<Value> argv[4];
							  pkg_cb_userlogin(data, argv);
//							  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
							  NanMakeCallback(NanGetCurrentContext()->Global(), NanNew(cIt->second), 4, argv);
							  break;
	}
	case T_ON_RSPUSERLOGOUT:
	{
							   Local<Value> argv[4];
							   pkg_cb_userlogout(data, argv);
//							   cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
							   NanMakeCallback(NanGetCurrentContext()->Global(), NanNew(cIt->second), 4, argv);
							   break;
	}
	case T_ON_RSPSUBMARKETDATA:
	{
								  Local<Value> argv[4];
								  pkg_cb_rspsubmarketdata(data, argv);
//								  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
								  NanMakeCallback(NanGetCurrentContext()->Global(), NanNew(cIt->second), 4, argv);
								  break;
	}
	case T_ON_RSPUNSUBMARKETDATA:
	{
									Local<Value> argv[4];
									pkg_cb_unrspsubmarketdata(data, argv);
//									cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
									NanMakeCallback(NanGetCurrentContext()->Global(), NanNew(cIt->second), 4, argv);
									break;
	}
	case T_ON_RTNDEPTHMARKETDATA:
	{
									Local<Value> argv[1];
									pkg_cb_rtndepthmarketdata(data, argv);
//									cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);
									NanMakeCallback(NanGetCurrentContext()->Global(), NanNew(cIt->second), 1, argv);
									break;
	}
	case T_ON_RSPERROR:
	{
						  Local<Value> argv[3];
						  pkg_cb_rsperror(data, argv);
//						  cIt->second->Call(Context::GetCurrent()->Global(), 3, argv);
						  NanMakeCallback(NanGetCurrentContext()->Global(), NanNew(cIt->second), 3, argv);

						  break;
	}
	}
	NanEscapeScope(NanUndefined());

}
void WrapMdUser::FunRtnCallback(int result, void* baton) {
	NanEscapableScope();
	LookupCtpApiBaton* tmp = static_cast<LookupCtpApiBaton*>(baton);
	if (tmp->uuid != -1) {
		std::map<const int, Persistent<Function> >::iterator it = fun_rtncb_map.find(tmp->uuid);
		Local<Value> argv[1] = { NanNew<Integer>(tmp->nResult) };
//		it->second->Call(Context::GetCurrent()->Global(), 1, argv);
		NanMakeCallback(NanGetCurrentContext()->Global(), NanNew(it->second), 1, argv);
		NanDisposePersistent(it->second);
		fun_rtncb_map.erase(tmp->uuid);
	}
	NanEscapeScope(NanUndefined());
}

void WrapMdUser::pkg_cb_userlogin(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = NanNew<Integer>(data->nRequestID);
	*(cbArray + 1) = NanNew<Boolean>(data->bIsLast)->ToBoolean();
	CThostFtdcRspUserLoginField* pRspUserLogin = static_cast<CThostFtdcRspUserLoginField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pRspUserLogin) {
		Local<Object> jsonRtn = NanNew<Object>();
		jsonRtn->Set(NanNew<String>("TradingDay"), NanNew<String>(pRspUserLogin->TradingDay));
		jsonRtn->Set(NanNew<String>("LoginTime"), NanNew<String>(pRspUserLogin->LoginTime));
		jsonRtn->Set(NanNew<String>("BrokerID"), NanNew<String>(pRspUserLogin->BrokerID));
		jsonRtn->Set(NanNew<String>("UserID"), NanNew<String>(pRspUserLogin->UserID));
		jsonRtn->Set(NanNew<String>("SystemName"), NanNew<String>(pRspUserLogin->SystemName));
		jsonRtn->Set(NanNew<String>("FrontID"), NanNew<Integer>(pRspUserLogin->FrontID));
		jsonRtn->Set(NanNew<String>("SessionID"), NanNew<Integer>(pRspUserLogin->SessionID));
		jsonRtn->Set(NanNew<String>("MaxOrderRef"), NanNew<String>(pRspUserLogin->MaxOrderRef));
		jsonRtn->Set(NanNew<String>("SHFETime"), NanNew<String>(pRspUserLogin->SHFETime));
		jsonRtn->Set(NanNew<String>("DCETime"), NanNew<String>(pRspUserLogin->DCETime));
		jsonRtn->Set(NanNew<String>("CZCETime"), NanNew<String>(pRspUserLogin->CZCETime));
		jsonRtn->Set(NanNew<String>("FFEXTime"), NanNew<String>(pRspUserLogin->FFEXTime));
		jsonRtn->Set(NanNew<String>("INETime"), NanNew<String>(pRspUserLogin->INETime));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = NanUndefined();
	}

	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapMdUser::pkg_cb_userlogout(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = NanNew<Integer>(data->nRequestID);
	*(cbArray + 1) = NanNew<Boolean>(data->bIsLast)->ToBoolean();
	CThostFtdcRspUserLoginField* pRspUserLogin = static_cast<CThostFtdcRspUserLoginField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pRspUserLogin) {
		Local<Object> jsonRtn = NanNew<Object>();
		jsonRtn->Set(NanNew<String>("BrokerID"), NanNew<String>(pRspUserLogin->BrokerID));
		jsonRtn->Set(NanNew<String>("UserID"), NanNew<String>(pRspUserLogin->UserID));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = NanUndefined();
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapMdUser::pkg_cb_rspsubmarketdata(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = NanNew<Integer>(data->nRequestID);
	*(cbArray + 1) = NanNew<Boolean>(data->bIsLast)->ToBoolean();
	CThostFtdcSpecificInstrumentField *pSpecificInstrument = static_cast<CThostFtdcSpecificInstrumentField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pSpecificInstrument) {
		Local<Object> jsonRtn = NanNew<Object>();
		jsonRtn->Set(NanNew<String>("InstrumentID"), NanNew<String>(pSpecificInstrument->InstrumentID));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = NanUndefined();
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapMdUser::pkg_cb_unrspsubmarketdata(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = NanNew<Integer>(data->nRequestID);
	*(cbArray + 1) = NanNew<Boolean>(data->bIsLast)->ToBoolean();
	CThostFtdcSpecificInstrumentField *pSpecificInstrument = static_cast<CThostFtdcSpecificInstrumentField*>(data->rtnField);
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	if (pSpecificInstrument) {
		Local<Object> jsonRtn = NanNew<Object>();
		jsonRtn->Set(NanNew<String>("InstrumentID"), NanNew<String>(pSpecificInstrument->InstrumentID));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = NanUndefined();
	}
	*(cbArray + 3) = pkg_rspinfo(pRspInfo);
	return;
}
void WrapMdUser::pkg_cb_rtndepthmarketdata(CbRtnField* data, Local<Value>*cbArray) {
	CThostFtdcDepthMarketDataField *pDepthMarketData = static_cast<CThostFtdcDepthMarketDataField*>(data->rtnField);
	if (pDepthMarketData) {	   		
		Local<Object> jsonRtn = NanNew<Object>();
		jsonRtn->Set(NanNew<String>("TradingDay"), NanNew<String>(pDepthMarketData->TradingDay));
		jsonRtn->Set(NanNew<String>("InstrumentID"), NanNew<String>(pDepthMarketData->InstrumentID));
		jsonRtn->Set(NanNew<String>("ExchangeID"), NanNew<String>(pDepthMarketData->ExchangeID));
		jsonRtn->Set(NanNew<String>("ExchangeInstID"), NanNew<String>(pDepthMarketData->ExchangeInstID));
		jsonRtn->Set(NanNew<String>("LastPrice"), NanNew<Number>(pDepthMarketData->LastPrice));
		jsonRtn->Set(NanNew<String>("PreSettlementPrice"), NanNew<Number>(pDepthMarketData->PreSettlementPrice));
		jsonRtn->Set(NanNew<String>("PreClosePrice"), NanNew<Number>(pDepthMarketData->PreClosePrice));
		jsonRtn->Set(NanNew<String>("PreOpenInterest"), NanNew<Number>(pDepthMarketData->PreOpenInterest));
		jsonRtn->Set(NanNew<String>("OpenPrice"), NanNew<Number>(pDepthMarketData->OpenPrice));
		jsonRtn->Set(NanNew<String>("HighestPrice"), NanNew<Number>(pDepthMarketData->HighestPrice));
		jsonRtn->Set(NanNew<String>("LowestPrice"), NanNew<Number>(pDepthMarketData->LowestPrice));
		jsonRtn->Set(NanNew<String>("Volume"), NanNew<Integer>(pDepthMarketData->Volume));
		jsonRtn->Set(NanNew<String>("Turnover"), NanNew<Number>(pDepthMarketData->Turnover));
		jsonRtn->Set(NanNew<String>("OpenInterest"), NanNew<Number>(pDepthMarketData->OpenInterest));
		jsonRtn->Set(NanNew<String>("ClosePrice"), NanNew<Number>(pDepthMarketData->ClosePrice));
		jsonRtn->Set(NanNew<String>("SettlementPrice"), NanNew<Number>(pDepthMarketData->SettlementPrice));
		jsonRtn->Set(NanNew<String>("UpperLimitPrice"), NanNew<Number>(pDepthMarketData->UpperLimitPrice));
		jsonRtn->Set(NanNew<String>("LowerLimitPrice"), NanNew<Number>(pDepthMarketData->LowerLimitPrice));
		jsonRtn->Set(NanNew<String>("PreDelta"), NanNew<Number>(pDepthMarketData->PreDelta));
		jsonRtn->Set(NanNew<String>("CurrDelta"), NanNew<Number>(pDepthMarketData->CurrDelta));
		jsonRtn->Set(NanNew<String>("UpdateTime"), NanNew<String>(pDepthMarketData->UpdateTime));
		jsonRtn->Set(NanNew<String>("UpdateMillisec"), NanNew<Integer>(pDepthMarketData->UpdateMillisec));
		jsonRtn->Set(NanNew<String>("BidPrice1"), NanNew<Number>(pDepthMarketData->BidPrice1));
		jsonRtn->Set(NanNew<String>("BidVolume1"), NanNew<Number>(pDepthMarketData->BidVolume1));
		jsonRtn->Set(NanNew<String>("AskPrice1"), NanNew<Number>(pDepthMarketData->AskPrice1));
		jsonRtn->Set(NanNew<String>("AskVolume1"), NanNew<Number>(pDepthMarketData->AskVolume1));
		jsonRtn->Set(NanNew<String>("BidPrice2"), NanNew<Number>(pDepthMarketData->BidPrice2));
		jsonRtn->Set(NanNew<String>("BidVolume2"), NanNew<Number>(pDepthMarketData->BidVolume2));
		jsonRtn->Set(NanNew<String>("AskPrice2"), NanNew<Number>(pDepthMarketData->AskPrice2));
		jsonRtn->Set(NanNew<String>("AskVolume2"), NanNew<Number>(pDepthMarketData->AskVolume2));
		jsonRtn->Set(NanNew<String>("BidPrice3"), NanNew<Number>(pDepthMarketData->BidPrice3));
		jsonRtn->Set(NanNew<String>("BidVolume3"), NanNew<Number>(pDepthMarketData->BidVolume3));
		jsonRtn->Set(NanNew<String>("AskPrice3"), NanNew<Number>(pDepthMarketData->AskPrice3));
		jsonRtn->Set(NanNew<String>("AskVolume3"), NanNew<Number>(pDepthMarketData->AskVolume3));
		jsonRtn->Set(NanNew<String>("BidPrice4"), NanNew<Number>(pDepthMarketData->BidPrice4));
		jsonRtn->Set(NanNew<String>("BidVolume4"), NanNew<Number>(pDepthMarketData->BidVolume4));
		jsonRtn->Set(NanNew<String>("AskPrice4"), NanNew<Number>(pDepthMarketData->AskPrice4));
		jsonRtn->Set(NanNew<String>("AskVolume4"), NanNew<Number>(pDepthMarketData->AskVolume4));
		jsonRtn->Set(NanNew<String>("BidPrice5"), NanNew<Number>(pDepthMarketData->BidPrice5));
		jsonRtn->Set(NanNew<String>("BidVolume5"), NanNew<Number>(pDepthMarketData->BidVolume5));
		jsonRtn->Set(NanNew<String>("AskPrice5"), NanNew<Number>(pDepthMarketData->AskPrice5));
		jsonRtn->Set(NanNew<String>("AskVolume5"), NanNew<Number>(pDepthMarketData->AskVolume5));
		jsonRtn->Set(NanNew<String>("AveragePrice"), NanNew<Number>(pDepthMarketData->AveragePrice));
		jsonRtn->Set(NanNew<String>("ActionDay"), NanNew<String>(pDepthMarketData->ActionDay));
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = NanUndefined();
	}
	
	return;
}  
void WrapMdUser::pkg_cb_rsperror(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = NanNew<Integer>(data->nRequestID);
	*(cbArray + 1) = NanNew<Boolean>(data->bIsLast)->ToBoolean();
	CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField*>(data->rspInfo);
	*(cbArray + 2) = pkg_rspinfo(pRspInfo);
	return;
}
Local<Value> WrapMdUser::pkg_rspinfo(CThostFtdcRspInfoField *pRspInfo) {
	if (pRspInfo) {
		Local<Object> jsonInfo = NanNew<Object>();
		jsonInfo->Set(NanNew<String>("ErrorID"), NanNew<Integer>(pRspInfo->ErrorID));
		jsonInfo->Set(NanNew<String>("ErrorMsg"), NanNew<String>(pRspInfo->ErrorMsg));
		return jsonInfo;
	}
	else {
		return 	NanUndefined();
	}
}
