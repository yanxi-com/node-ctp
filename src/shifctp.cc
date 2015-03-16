#include <node.h>
#include <v8.h>
#include "wrap_trader.h"
#include "wrap_mduser.h"

using namespace v8;

bool islog;//log?

NAN_METHOD(CreateTrader) {
	NanScope();
	NanReturnValue(NanNew<FunctionTemplate>(WrapTrader::NewInstance)->GetFunction());
}

NAN_METHOD(CreateMdUser) {
	NanScope();
	NanReturnValue(NanNew<FunctionTemplate>(WrapMdUser::NewInstance)->GetFunction());
}

NAN_METHOD(Settings) {
	NanScope();

	if (!args[0]->IsUndefined() && args[0]->IsObject()) {
		Local<Object> setting = args[0]->ToObject();
		Local<Value> log = setting->Get(NanNew<String>("log"));
		if (!log->IsUndefined()) {
			islog = log->BooleanValue();
		}
	}

	NanReturnValue(NanUndefined());
}

void Init(Handle<Object> exports) {
	WrapTrader::Init(0);
	WrapMdUser::Init(0);
	exports->Set(NanNew<String>("createTrader"),
		NanNew<FunctionTemplate>(CreateTrader)->GetFunction());
	exports->Set(NanNew<String>("createMdUser"),
		NanNew<FunctionTemplate>(CreateMdUser)->GetFunction());
	exports->Set(NanNew<String>("settings"),
		NanNew<FunctionTemplate>(Settings)->GetFunction());
}

NODE_MODULE(shifctp, Init)
