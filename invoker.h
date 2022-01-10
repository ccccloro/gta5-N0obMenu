#pragma once

#include <Windows.h>
#include <inttypes.h>
#include "config.h"

typedef struct SInvokerCxt
{
	uint64_t* retVal;
	uint64_t argCount = 0;
	uint64_t* stackPtr;
	uint64_t dataCount = 0;
	uint64_t spaceForResults[24];
	uint64_t stack[128];

} invoker_context;

using NativeHandler = void(*)(SInvokerCxt*);
static void(*SetVectorsResult)(SInvokerCxt*);

static NativeHandler(*GetNativeHandler)(void*, uint64_t hash);
static void* NativeRegistrationTable;

static __forceinline void resetArgs(SInvokerCxt* ctx)
{
	ctx->argCount = 0;
	ctx->dataCount = 0;
}

template<typename T>
static __forceinline void pushArg(T value, SInvokerCxt* ctx) {
	*(T*)&ctx->stack[ctx->argCount++] = value;
}

template<class T>
static __forceinline T getReturn(SInvokerCxt* ctx) {
	return *reinterpret_cast<T*>(ctx->retVal);
}

template<typename N, typename... A>
static N invoke(LPVOID entrypoint, A... args)
{
	SInvokerCxt ctx;
	ctx.retVal = ctx.stack;
	ctx.stackPtr = ctx.stack;

	resetArgs(&ctx);
	int dummy[] = { 0, ((void)pushArg(std::forward<A>(args), &ctx), 0) ... };

	auto handler = (NativeHandler)entrypoint;
	if (handler)
		x64::spoof_call(handler, &x64::fastcall_ctx, &ctx);

	x64::spoof_call(SetVectorsResult, &x64::fastcall_ctx, &ctx);
	return getReturn<N>(&ctx);
}