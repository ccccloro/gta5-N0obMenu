#include <Windows.h>
#include <Psapi.h>
#include <fstream>
#include <iostream>
#include "log.h"
#include "memscan.h"
#include "MinHook.h"
#include "module.h"
#include "hooking.h"
#include "config.h"
#include "events.h"

#pragma comment(lib, "Ws2_32.lib")

typedef struct _MANUAL_INJECT
{
	typedef HMODULE(WINAPI* pLoadLibraryA)(LPCSTR);
	typedef FARPROC(WINAPI* pGetProcAddress)(HMODULE, LPCSTR);

	PVOID ImageBase;
	PIMAGE_NT_HEADERS NtHeaders;
	PIMAGE_BASE_RELOCATION BaseRelocation;
	PIMAGE_IMPORT_DESCRIPTOR ImportDirectory;
	pLoadLibraryA fnLoadLibraryA;
	pGetProcAddress fnGetProcAddress;
	PVOID injDataAddress;
	PVOID ShellcodeAddress;
	size_t ShellcodeSize;

}MANUAL_INJECT, * PMANUAL_INJECT;

LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
	PVOID ExceptionAddress = ExceptionInfo->ExceptionRecord->ExceptionAddress;
	PVOID ExceptionOffset = (PVOID)((DWORD_PTR)ExceptionInfo->ExceptionRecord->ExceptionAddress - (DWORD_PTR)gta5::baseAddress);

	std::ofstream repfile("crash-report.txt");
	repfile << "Noobmenu crash report" << std::endl << std::endl;
	repfile << "ExceptionAddress: " << ExceptionAddress << std::endl;
	repfile << "Rip: " << (PVOID)ExceptionInfo->ContextRecord->Rip << std::endl;
	repfile << "ExceptionAddress (offset): " << ExceptionOffset << std::endl;
	repfile << "Last script event hash received: " << (int)lastScriptEventHashReceived << std::endl;
	repfile << "Last sync model hash received: " << (int)lastSyncModelHashReceived << std::endl;
	repfile.close();

	return EXCEPTION_CONTINUE_SEARCH;
}

BOOL InitFastcallSpoofer()
{
	MODULEINFO module_info;
	if (K32GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &module_info, sizeof(MODULEINFO)) == 0)
	{
		cmd::write(RED, "unexpected exception - retrieving modules info");
		return FALSE;
	}

	gta5::baseAddress = module_info.lpBaseOfDll;
	gta5::moduleSize = module_info.SizeOfImage;

#ifdef DEBUG
	cmd::write(GRAY, "[DEBUG] lpBaseOfProcess: %p", gta5::baseAddress);
	cmd::write(GRAY, "[DEBUG] SizeOfMemorySpace: %p", gta5::moduleSize);
#endif

	mem::module_ctx scan_ctx;
	scan_ctx.start = (DWORD_PTR)module_info.lpBaseOfDll;
	scan_ctx.end = ((DWORD_PTR)module_info.lpBaseOfDll + module_info.SizeOfImage);

	//init call spoofer
	LPVOID rbp_gadget = mem::find(s_decrypt(patterns::rdi).c_str(), &scan_ctx).get_addr();
	if (rbp_gadget == NULL)
	{
		cmd::write(DARKYELLOW, "signature - JRDI");
		return FALSE;
	}
	cmd::write(GRAY, "signature - JRDI");

	size_t spoofer_length = sizeof(spoofer_payload) + sizeof(spoofer_cleanup);
	x64::spoofer_size = spoofer_length;

	x64::spoofer_cave = NULL;
	if ((x64::spoofer_cave = VirtualAlloc(NULL, spoofer_length, MEM_COMMIT, PAGE_EXECUTE_READWRITE)) == NULL)
	{
		cmd::write(RED, "unexpected exception - allocating memory for SPFR buffer");
		return FALSE;
	}
	cmd::write(WHITE, "Allocated memory for SPFR buffer");

#ifdef DEBUG
	cmd::write(WHITE, "%p", x64::spoofer_cave);
#endif

	memcpy(x64::spoofer_cave, spoofer_payload, sizeof(spoofer_payload));
	memcpy((LPVOID)((DWORD_PTR)x64::spoofer_cave + sizeof(spoofer_payload)), spoofer_cleanup, sizeof(spoofer_cleanup));
	memset(spoofer_payload, 0, sizeof(spoofer_payload));
	memset(spoofer_cleanup, 0, sizeof(spoofer_cleanup));

	DWORD_PTR cleanup_address = (DWORD_PTR)x64::spoofer_cave + sizeof(spoofer_payload);
	for (DWORD i = 0; i < sizeof(spoofer_payload); i++)
	{
		if (*reinterpret_cast<PBYTE>((LPVOID)((DWORD_PTR)x64::spoofer_cave + i)) == (BYTE)0xCC)
		{
			memcpy((LPVOID)((DWORD_PTR)x64::spoofer_cave + i), &cleanup_address, sizeof(DWORD_PTR));
			break;
		}
	}

	x64::fastcall_ctx.gadget = rbp_gadget;
	x64::fastcall_ctx.payload = x64::spoofer_cave;

	return TRUE;
}

VOID FreeDll(HMODULE mod, DWORD cReason)
{
	if (MH_DisableHook(MH_ALL_HOOKS) == MH_OK)
		cmd::write(DARKYELLOW, "Disabled all hooks");

	if (MH_RemoveHook(MH_ALL_HOOKS) == MH_OK)
		cmd::write(DARKYELLOW, "Removed all hooks");

	if (MH_Uninitialize() == MH_OK)
		cmd::write(DARKYELLOW, "Uninitialized MH");

	CFrontend::ReleaseAllFrontend();

	if (x64::spoofer_cave)
	{
		if (VirtualFree(x64::spoofer_cave, NULL, MEM_RELEASE))
			cmd::write(DARKYELLOW, "Released SPFR buffer");
	}

#ifndef MANUAL_MAP
	if (cReason == DLL_PROCESS_ATTACH) FreeLibraryAndExitThread(mod, NULL);
	if (cReason == DLL_PROCESS_DETACH) FreeLibrary(mod);
#endif
}

DWORD WINAPI MainConsole(LPVOID par)
{
	if (AllocConsole())
		freopen("CONOUT$", "w", stdout);

	system("cls");
	cmd::write(GRAY, "Welcome to Noobmenu! version 2.0 (gta5)");

#ifdef MANUAL_MAP

	PMANUAL_INJECT ManualInject = NULL;
	if (par == NULL)
	{
		cmd::write(RED, "Please inject with the proper injector!");
		FreeDll((HMODULE)par, DLL_PROCESS_ATTACH);
		return TRUE;
	}

	ManualInject = (PMANUAL_INJECT)par;

	//cleanup injection data
	PVOID shellCodeAddr = NULL;
	PVOID injDataAddress = NULL;

	memset(ManualInject->ShellcodeAddress, 0, ManualInject->ShellcodeSize);
	shellCodeAddr = ManualInject->ShellcodeAddress;
	injDataAddress = ManualInject->injDataAddress;
	memset(ManualInject->injDataAddress, 0, 4096 /* injector allocates 4096 bytes*/);

	if (!VirtualFree(injDataAddress, NULL, MEM_RELEASE))
	{
		cmd::write(RED, "fatal error - could not release injection data 1");
		FreeDll((HMODULE)par, DLL_PROCESS_ATTACH);
		return FALSE;
	}

	if (!VirtualFree(shellCodeAddr, NULL, MEM_RELEASE))
	{
		cmd::write(RED, "fatal error - could not release injection data 2");
		FreeDll((HMODULE)par, DLL_PROCESS_ATTACH);
		return FALSE;
	}

#endif


	auto hwnd = FindWindowW(0, L"Grand Theft Auto V");
	if (hwnd == NULL)
	{
		cmd::write(CYAN, "please, inject this client into GTA5 game");
		FreeDll((HMODULE)par, DLL_PROCESS_ATTACH);
		return FALSE;
	}
	if (!GetWindowRect(hwnd, &CGraphicsMgr::srScreenSize))
	{
		cmd::write(RED, "Unexpected error - please close the game");
		FreeDll((HMODULE)par, DLL_PROCESS_ATTACH);
		return FALSE;
	}

#ifdef DEBUG
	cmd::write(GRAY, "screen-w: %d", graphics::screen_size.right);
	cmd::write(GRAY, "screen-h: %d", graphics::screen_size.bottom);
#endif

	if (!InitFastcallSpoofer())
	{
		FreeDll((HMODULE)par, DLL_PROCESS_ATTACH);
		return FALSE;
	}

	mem::module_ctx scan_ctx;
	scan_ctx.start = (DWORD_PTR)gta5::baseAddress;
	scan_ctx.end = ((DWORD_PTR)gta5::baseAddress + gta5::moduleSize);

	LPVOID location = NULL;

	//GetNativeHandler
	if ((location = mem::find(s_decrypt(patterns::NativesRegistrationTable).c_str(), &scan_ctx).add(11)->get_call()) == NULL)
	{
		cmd::write(DARKYELLOW, "Signature - GNH");
		return FALSE;
	}
	cmd::write(GRAY, "Signature - GNH");
	GetNativeHandler = (NativeHandler(*)(void*, uint64_t))location;
	location = NULL;

	//NativesRegistrationTable
	if ((location = mem::find(s_decrypt(patterns::NativesRegistrationTable).c_str(), &scan_ctx).add(3)->rip()->get_addr()) == NULL)
	{
		cmd::write(DARKYELLOW, "Signature - NRT");
		return FALSE;
	}
	cmd::write(GRAY, "Signature - NRT");
	NativeRegistrationTable = location;
	location = NULL;

	//SetVectorsResults
	if ((location = mem::find(s_decrypt(patterns::set_vectors_results).c_str(), &scan_ctx).get_addr()) == NULL)
	{
		cmd::write(YELLOW, "Signature - SVR");
		FreeDll((HMODULE)par, DLL_PROCESS_ATTACH);
		return FALSE;
	}
	cmd::write(GRAY, "Signature - SVR", location);
	SetVectorsResult = (void(*)(SInvokerCxt*))location;

	//cmd::write(CYAN, "PLSFE: %p", GetNativeHandler(NativeRegistrationTable, 0xD9BD5965B9552645));

	if (MH_Initialize() != MH_OK)
	{
		cmd::write(RED, "unexpected exception - initializing hooking core");
		FreeDll((HMODULE)par, DLL_PROCESS_ATTACH);
		return FALSE;
	}

	//finds natives at runtime and init all hooks, then start the script
	if (CHookMgr::init_hooks())
	{
		CHookMgr::enable_mainhook(TRUE);

#ifdef EX_MODE
		HANDLE hCrashHandler = AddVectoredExceptionHandler(true, CrashHandler);
		if (!hCrashHandler)
			cmd::write(RED, "Could not register a debugging crash handler!");
#endif
	}
	else
	{
		FreeDll(NULL, NULL);
		return FALSE;
	}
	return TRUE;
}

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD cReason, LPVOID reserved)
{
	if (cReason == DLL_PROCESS_ATTACH)
	{
		HANDLE hThread = NULL;
		if ((hThread = CreateThread(NULL, NULL, MainConsole, reserved, NULL, NULL)) != NULL)
			CloseHandle(hThread);
		return TRUE;
	}
	return TRUE;
}