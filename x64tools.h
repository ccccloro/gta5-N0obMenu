#pragma once

#include <Windows.h>
#include <iostream>

static BYTE spoofer_cleanup[] =
{
	0x48, 0x83, 0xEC, 0x10, 0x4C,
	0x8B, 0x5F, 0x08, 0x48, 0x8B,
	0x7F, 0x10, 0x41, 0xFF, 0xE3
};
/*
* 
sub rsp, 0x8
mov r11, [rdi+0x8]
mov rdi, [rdi+0x10]
jmp r11
*/

/*static BYTE spoofer_cleanup[] =
{
	 0x4C, 0x8B, 0x5F, 0x08, 
	 0x48, 0x8B, 0x7F, 0x10, 
	 0x48, 0x83, 0xEC, 0x10, 
	 0x41, 0xFF, 0xE3 
};*/

/*
* 
mov r11, [rdi+0x8]
mov rdi, [rdi+0x10]
sub rsp, 0x8
jmp r11

*/

static BYTE spoofer_payload[] =
{
	0x41, 0x5A, 0x48, 0x83, 0xC4, 0x08, 0x48, 
	0x89, 0xE0, 0x48, 0x83, 0xC0, 0x18, 0x48, 
	0x8B, 0x00, 0x4C, 0x8B, 0x18, 0x4C, 0x89, 
	0x1C, 0x24, 0x48, 0x89, 0x78, 0x10, 0x48, 
	0x89, 0xC7, 0x4C, 0x8B, 0x58, 0x08, 0x4C, 
	0x89, 0x50, 0x08, 0x49, 0xBA, 0xCC, 0xCC, 
	0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x4C, 
	0x89, 0x10, 0x41, 0xFF, 0xE3
};

/*

pop r10
add rsp,0x8
mov rax, rsp
add rax, 0x18
mov rax, [rax]
mov r11, [rax]
mov [rsp], r11
mov [rax + 0x10], rdi
mov rdi, rax
mov r11, [rax + 0x8]
mov [rax + 0x8],r10
movabs r10, 0xCCCCCCCCCCCCCCCC
mov [rax], r10
jmp r11

*/

typedef struct _x64_fastcall_spoof_ctx
{
	LPVOID payload;
	LPVOID gadget;

} x64_fastcall_spoof_ctx;

class x64
{
private:

	template<typename Ret, typename...Args>
	static inline auto call_payload(x64_fastcall_spoof_ctx* call_ctx, Args...args) -> Ret
	{
		auto pf = (Ret(*)(Args...))((LPVOID)call_ctx->payload);
		return pf(args...);
	}

	//5 or more args
	template<size_t Argc, typename>
	struct args_remapper
	{
		template<typename Ret, class First, class Second, class Third, class Fourth, typename...Pack>
		static auto do_call(x64_fastcall_spoof_ctx* call_ctx, LPVOID p_params, First first, Second second, Third third, Fourth fourth, Pack...pack) -> Ret
		{
			return call_payload<Ret, First, Second, Third, Fourth, void*, void*, Pack...>(call_ctx, first, second, third, fourth, p_params, nullptr, pack...);
		}
	};

	//only for 4 args or less
	template<size_t Argc>
	struct args_remapper<Argc, std::enable_if_t<Argc <= 4>>
	{
		template<typename Ret, class First = LPVOID, class Second = LPVOID, class Third = LPVOID, class Fourth = LPVOID>
		static auto do_call(x64_fastcall_spoof_ctx* call_ctx, LPVOID p_params, First first = First{},
			Second second = Second{}, Third third = Third{}, Fourth fourth = Fourth{}) -> Ret
		{
			return call_payload<Ret, First, Second, Third, Fourth, LPVOID, void*>(call_ctx, first, second, third, fourth, p_params, nullptr);
		}
	};

public:

	template<typename Ret, typename...Args>
	static inline auto spoof_call(Ret(*fn)(Args...), x64_fastcall_spoof_ctx* call_ctx, Args...args) -> Ret
	{
		struct p_params
		{
			LPVOID gadget;
			LPVOID fn;
			PULONG64 rbp;
		};

		p_params params;
		params.gadget = call_ctx->gadget;
		params.fn = static_cast<LPVOID>(fn);

		using remapper = args_remapper<sizeof...(Args), void>;
		return remapper::template do_call<Ret, Args...>(call_ctx, (LPVOID)&params, args...);
	}

public:

	static inline x64_fastcall_spoof_ctx fastcall_ctx;
	static inline LPVOID spoofer_cave = NULL;
	static inline size_t spoofer_size = NULL;
};
