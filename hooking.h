#pragma once

#include <Windows.h>
#include <inttypes.h>
#include "types.h"
#include "memscan.h"
#include "strings.h"
#include "module.h"
#include "log.h"
#include "game.h"
#include "script.h"
#include "scrypt.h"
#include "events.h"

//main hook for script
static fpDoesCamExists og_does_cam_exists = NULL;
static char __cdecl hk_does_cam_exists(Cam cam)
{
	static uint64_t lastTickCount = 0;
	if (lastTickCount != *CMemoryMgr::tick_count)
	{
		if (!script::bHasSetupRun)
		{
			script::setup();
			script::bHasSetupRun = TRUE;
		}

		script::tick();
		lastTickCount = *CMemoryMgr::tick_count;
	}
	return x64::spoof_call(og_does_cam_exists, &x64::fastcall_ctx, cam);
}

class CHookMgr
{
public:

	static VOID vhook(__int64 vTable, __int64 fp, int index)
	{
		*(__int64*)(vTable + index * 8) = fp;
	}

	static BOOL init_hooks()
	{
		if (!FindMemory())
			return FALSE;

		if (!FindNatives())
			return FALSE;

		if (!FindAndHookEvents())
			return FALSE;

		if (!CNativesMgr::does_cam_exists)
			return FALSE;

		if (MH_CreateHook(CNativesMgr::does_cam_exists, hk_does_cam_exists, (void**)&og_does_cam_exists) != MH_OK)
		{
			cmd::write(RED, "Hook - MSH");
			return FALSE;
		}
		if (MH_EnableHook(CNativesMgr::does_cam_exists) != MH_OK)
		{
			cmd::write(RED, "Hook - MSH");
			return FALSE;
		}

		cmd::write(GRAY, "Hook - MSH");
		return TRUE;
	}

	static BOOL enable_mainhook(BOOL toggle)
	{
		if (toggle)
		{
			if (MH_EnableHook(CNativesMgr::does_cam_exists) == MH_OK)
				return TRUE;
		}
		else
		{
			if (MH_DisableHook(CNativesMgr::does_cam_exists) == MH_OK)
				return TRUE;
		}
		return FALSE;
	}

private:

	CHookMgr() {};
	~CHookMgr() {};

	static BOOL FindMemory()
	{
		LPVOID location = NULL;

		mem::module_ctx srScanCxt;
		srScanCxt.start = (DWORD_PTR)gta5::baseAddress;
		srScanCxt.end = (DWORD_PTR)gta5::baseAddress + gta5::moduleSize;

		//tick_count (uint64_t)
		if ((location = mem::find(s_decrypt(patterns::tick_count).c_str(), &srScanCxt).add(2)->rip()->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - TKC");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - TKC");
		CMemoryMgr::tick_count = (uint32_t*)location;
		location = NULL;

		//text_info
		if ((location = mem::find(s_decrypt(patterns::text_info).c_str(), &srScanCxt).add(3)->rip()->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - TXTI");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - TXTI", location);
		CGraphicsMgr::pTextInfo = (CTextInfo*)location;
		location = NULL;

		//pCNetworkPlayerMgr
		if ((location = mem::find(s_decrypt(patterns::pCNetworkPlayerMgr).c_str(), &srScanCxt).add(3)->rip()->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - CNPMGR");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - CNPMGR", location);
		CMemoryMgr::pCNetworkPlayerMgr = (__int64*)location;
		location = NULL;

		//GetNetGamePlayerFromId
		if ((location = mem::find(s_decrypt(patterns::GetNetGamePlayerFromId).c_str(), &srScanCxt).get_call()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - GNGP");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - GNGP");
		CMemoryMgr::pGetNetPlayerFromId = (fpGetNetGamePlayerFromId)location;
		location = NULL;

		//GetPedClass
		if ((location = mem::find(s_decrypt(patterns::GetPedClass).c_str(), &srScanCxt).get_call()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - GEC");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - GEC");
		CMemoryMgr::pGetPedClassFromId = (fpGetPedClass)location;
		location = NULL;

		//GetEntityFromClass
		if ((location = mem::find(s_decrypt(patterns::GetEntityFromClass).c_str(), &srScanCxt).get_call()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - GEFC");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - GEFC");
		CMemoryMgr::pGetEntityFromClass = (fpGetEntityFromClass)location;
		location = NULL;

		//read_bitbuffer_dword
		if ((location = mem::find(s_decrypt(patterns::read_bitbuffer_dword).c_str(), &srScanCxt).add(-5)->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - RBDW");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - RBDW");
		CMemoryMgr::read_bitbuffer_dword = (fpReadBitbufferDWORD)location;
		location = NULL;

		//read_bitbuffer_word
		if ((location = mem::find(s_decrypt(patterns::read_bitbuffer_word).c_str(), &srScanCxt).add(-5)->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - RBW");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - RBW");
		CMemoryMgr::read_bitbuffer_word = (fpReadBitbufferWORD)location;
		location = NULL;

		return TRUE;
	}

	static BOOL FindAndHookEvents() /* some network event handlers are found within the vtable section */
	{
		LPVOID location = NULL;

		mem::module_ctx srScanCxt;
		srScanCxt.start = (DWORD_PTR)gta5::baseAddress;
		srScanCxt.end = (DWORD_PTR)gta5::baseAddress + gta5::moduleSize;

		//scripted_event
		if ((location = mem::find(s_decrypt(patterns::scripted_event).c_str(), &srScanCxt).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - SCRE");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - SCRE", location);
		CNetPtrMgr::scripted_event = (fpScriptedEvent)location;
		location = NULL;

		if (MH_CreateHook(CNetPtrMgr::scripted_event, hk_scripted_event, (void**)&og_scripted_event) != MH_OK)
		{
			cmd::write(RED, "Hook - SCRE");
			return FALSE;
		}
		if (MH_EnableHook(CNetPtrMgr::scripted_event) != MH_OK)
		{
			cmd::write(RED, "Hook - SCRE");
			return FALSE;
		}
		cmd::write(GRAY, "Hook - SCRE", CNetPtrMgr::scripted_event);

		//clear_ped_task_event
		if ((location = mem::find(s_decrypt(patterns::clear_ped_task_event).c_str(), &srScanCxt).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - CPTE");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - CPTE", location);
		CNetPtrMgr::clear_ped_task_event = (fpClearPedTaskEvent)location;
		location = NULL;

		if (MH_CreateHook(CNetPtrMgr::clear_ped_task_event, hk_clear_ped_task_event, (void**)&og_clear_ped_task_event) != MH_OK)
		{
			cmd::write(RED, "Hook - CPTE");
			return FALSE;
		}
		if (MH_EnableHook(CNetPtrMgr::clear_ped_task_event) != MH_OK)
		{
			cmd::write(RED, "Hook - CPTE");
			return FALSE;
		}
		cmd::write(GRAY, "Hook - CPTE", CNetPtrMgr::clear_ped_task_event);
		location = NULL;

		//increment_stat_event
		if ((location = mem::find(s_decrypt(patterns::increment_stat_event).c_str(), &srScanCxt).add(-10)->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - ISE");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - ISE", location);
		CNetPtrMgr::increment_stat_event = (fpIncrementStatEvent)location;

		if (MH_CreateHook(CNetPtrMgr::increment_stat_event, hk_increment_stat_event, (void**)&og_increment_stat_event) != MH_OK)
		{
			cmd::write(RED, "Hook - ISE");
			return FALSE;
		}
		if (MH_EnableHook(CNetPtrMgr::increment_stat_event) != MH_OK)
		{
			cmd::write(RED, "Hook - ISE");
			return FALSE;
		}
		cmd::write(GRAY, "Hook - ISE", CNetPtrMgr::increment_stat_event);
		location = NULL;

		//request_control_event
		if ((location = mem::find(s_decrypt(patterns::request_control_event).c_str(), &srScanCxt).add(-10)->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - RCTRLE");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - RCTRLE", location);
		CNetPtrMgr::request_control_event = (fpRequestControlEvent)location;

		if (MH_CreateHook(CNetPtrMgr::request_control_event, hk_request_control_event, (void**)&og_request_control_event) != MH_OK)
		{
			cmd::write(RED, "Hook - RCTRLE");
			return FALSE;
		}
		if (MH_EnableHook(CNetPtrMgr::request_control_event) != MH_OK)
		{
			cmd::write(RED, "Hook - RCTRLE");
			return FALSE;
		}
		cmd::write(GRAY, "Hook - RCTRLE", CNetPtrMgr::request_control_event);
		location = NULL;

		//info_change_event
		if ((location = mem::find(s_decrypt(patterns::info_change_event).c_str(), &srScanCxt).add(-15)->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - ICE");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - ICE", location);
		CNetPtrMgr::info_change_event = (fpInfoChangeEvent)location;
		location = NULL;

		if (MH_CreateHook(CNetPtrMgr::info_change_event, hk_info_change_event, (void**)&og_info_change_event) != MH_OK)
		{
			cmd::write(RED, "Hook - ICE");
			return FALSE;
		}
		if (MH_EnableHook(CNetPtrMgr::info_change_event) != MH_OK)
		{
			cmd::write(RED, "Hook - ICE");
			return FALSE;
		}
		cmd::write(GRAY, "Hook - ICE", CNetPtrMgr::info_change_event);

		//crc_hash_check_event
		if ((location = mem::find(s_decrypt(patterns::crc_hash_check_event).c_str(), &srScanCxt).add(-10)->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - CHCE");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - CHCE", location);
		CNetPtrMgr::crc_hash_check_event = (fpCrcHashCheckEvent)location;
		location = NULL;

		//check_exe_size_event
		if ((location = mem::find(s_decrypt(patterns::check_exe_size_event).c_str(), &srScanCxt).add(-15)->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - CESE");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - CESE", location);
		CNetPtrMgr::check_exe_size_event = (fpCheckExeSizeEvent)location;
		location = NULL;

		//trigger_script_event
		if ((location = mem::find(s_decrypt(patterns::trigger_script_event).c_str(), &srScanCxt).add(-15)->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - TSE");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - TRSE", location);
		CNetPtrMgr::trigger_script_event = (fpTriggerScriptEvent)location;
		location = NULL;

		//hook network events vtables with a return 0 function to prevent the from being triggered
		//find a gadget

		LPVOID ret0 = NULL;
		if ((ret0 = mem::find(s_decrypt(patterns::ret0).c_str(), &srScanCxt).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - RETG");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - RETG", location);

		//vtInfoChangeEvent
		if ((location = mem::find(s_decrypt(patterns::vtInfoChangeEvent).c_str(), &srScanCxt).add(3)->rip()->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - RAC_1");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - RAC_1", location);
		vhook((__int64)location, (__int64)ret0, 2);
		cmd::write(GRAY, "Hook - RAC_1", location);
		location = NULL;

		//vtCheckCrcHashEvent
		if ((location = mem::find(s_decrypt(patterns::vtCheckCrcHashEvent).c_str(), &srScanCxt).add(3)->rip()->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - RAC_2");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - RAC_2", location);
		vhook((__int64)location, (__int64)ret0, 2);
		cmd::write(GRAY, "Hook - RAC_2", location);
		location = NULL;

		//vtExplosionEvent
		if ((location = mem::find(s_decrypt(patterns::vtExplosionEvent).c_str(), &srScanCxt).add(3)->rip()->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - EXPLE");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - EXPLE", location);
		CNetPtrMgr::explosion_event = (fpExplosionEvent)(*(__int64*)((DWORD_PTR)location + 56));
		location = NULL;

		if (MH_CreateHook(CNetPtrMgr::explosion_event, hk_explosion_event, (void**)&og_explosion_event) != MH_OK)
		{
			cmd::write(RED, "Hook - EXPLE");
			return FALSE;
		}
		if (MH_EnableHook(CNetPtrMgr::explosion_event) != MH_OK)
		{
			cmd::write(RED, "Hook - EXPLE");
			return FALSE;
		}
		cmd::write(GRAY, "Hook - EXPLE", CNetPtrMgr::explosion_event);

		//vtRemoveWeaponEvent
		if ((location = mem::find(s_decrypt(patterns::vtRemoveWeaponEvent).c_str(), &srScanCxt).add(3)->rip()->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - RWE");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - RWE", location);
		CNetPtrMgr::remove_weapon_event = (fpRemoveWeaponEvent)(*(__int64*)((DWORD_PTR)location + 48));
		location = NULL;

		if (MH_CreateHook(CNetPtrMgr::remove_weapon_event, hk_remove_weapon_event, (void**)&og_remove_weapon_event) != MH_OK)
		{
			cmd::write(RED, "Hook - RWE");
			return FALSE;
		}
		if (MH_EnableHook(CNetPtrMgr::remove_weapon_event) != MH_OK)
		{
			cmd::write(RED, "Hook - RWE");
			return FALSE;
		}
		cmd::write(GRAY, "Hook - RWE", CNetPtrMgr::remove_weapon_event);

		//vtCheckExeSizeEvent
		if ((location = mem::find(s_decrypt(patterns::vtCheckExeSizeEvent).c_str(), &srScanCxt).add(3)->rip()->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - RAC_3");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - RAC_3", location);
		vhook((__int64)location, (__int64)ret0, 2);
		cmd::write(GRAY, "Hook - RAC_3", location);
		location = NULL;

		//vtReportMyselfEvent
		if ((location = mem::find(s_decrypt(patterns::vtReportMyselfEvent).c_str(), &srScanCxt).add(7)->rip()->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - RAC_4");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - RAC_4", location);
		vhook((__int64)location, (__int64)ret0, 2);
		cmd::write(GRAY, "Hook - RAC_4", location);
		location = NULL;

		/* CGameScriptHandlerNetComponent vtable */

		//vCGameScriptHandlerNetComponent
		if ((location = mem::find(s_decrypt(patterns::vtCGameScriptHandlerNetComponent).c_str(), &srScanCxt).add(3)->rip()->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - GSHNC");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - GSHNC", location);
		CVtableMgr::vtCGameScriptHandlerNetComponent = (__int64)location;
		CNetPtrMgr::MigrateScriptHost = (fpMigrateScriptHost)(*(__int64*)((__int64)location + (31 * 8)));
		location = NULL;

		//MigrateScriptHost
		if (MH_CreateHook(CNetPtrMgr::MigrateScriptHost, hk_migrate_script_host, (void**)&og_migrate_script_host) != MH_OK)
		{
			cmd::write(RED, "Hook - MSCH");
			return FALSE;
		}
		if (MH_EnableHook(CNetPtrMgr::MigrateScriptHost) != MH_OK)
		{
			cmd::write(RED, "Hook - MSCH");
			return FALSE;
		}
		cmd::write(GRAY, "Hook - MSCH", CNetPtrMgr::MigrateScriptHost);

		//vtCPlane
		if ((location = mem::find(s_decrypt(patterns::vtCPlane).c_str(), &srScanCxt).add(3)->rip()->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - GSHNC");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - GSHNC", location);
		CVtableMgr::vtCPlane = (__int64)location;
		CNetPtrMgr::SendNetObjPlane = (fpSendNetObjPlane)(*(__int64*)((__int64)location + (99 * 8)));
		location = NULL;

		//SendNetObjPlane
		/*
		if (MH_CreateHook(CNetPtrMgr::SendNetObjPlane, hk_SendNetObjPlane, (void**)&og_SendNetObjPlane) != MH_OK)
		{
			cmd::write(RED, "Hook - SNOP");
			return FALSE;
		}
		if (MH_EnableHook(CNetPtrMgr::SendNetObjPlane) != MH_OK)
		{
			cmd::write(RED, "Hook - SNOP");
			return FALSE;
		}
		cmd::write(GRAY, "Hook - SNOP", CNetPtrMgr::SendNetObjPlane);
		*/

		//sub_7FF7AA059CC4
		if ((location = mem::find(s_decrypt(patterns::sub_7FF7AA059CC4).c_str(), &srScanCxt).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - SUB");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - SUB", location);
		CNetPtrMgr::sub_7FF7AA059CC4 = (fpSub7FF7AA059CC4)location;
		location = NULL;

		if (MH_CreateHook(CNetPtrMgr::sub_7FF7AA059CC4, hk_sub_7FF7AA059CC4, (void**)&og_sub_7FF7AA059CC4) != MH_OK)
		{
			cmd::write(RED, "Hook - SUB");
			return FALSE;
		}
		if (MH_EnableHook(CNetPtrMgr::sub_7FF7AA059CC4) != MH_OK)
		{
			cmd::write(RED, "Hook - SUB");
			return FALSE;
		}
		cmd::write(GRAY, "Hook - SUB", CNetPtrMgr::sub_7FF7AA059CC4);


		//vtCNetworkObjMgr (find sync trees)
		if ((location = mem::find(s_decrypt(patterns::vtCNetworkObjectMgr).c_str(), &srScanCxt).add(3)->rip()->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - NOMV");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - NOMV", location);
		CVtableMgr::vtCNetworkObjectMgr = (__int64)location;
		CNetPtrMgr::received_clone_sync = (fpReceivedCloneSync)(*(__int64*)((__int64)location + (21 * 8)));
		CNetPtrMgr::received_clone_create = (fpReceivedCloneCreate)(*(__int64*)((__int64)location + (19 * 8)));
		CNetPtrMgr::received_clone_create_ack = (fpReceivedCloneCreateAck)(*(__int64*)((__int64)location + (20 * 8)));
		location = NULL;

		//received_clone_sync
		if (MH_CreateHook(CNetPtrMgr::received_clone_sync, hk_received_clone_sync, (void**)&og_received_clone_sync) != MH_OK)
		{
			cmd::write(RED, "Hook - SYNT_S");
			return FALSE;
		}
		if (MH_EnableHook(CNetPtrMgr::received_clone_sync) != MH_OK)
		{
			cmd::write(RED, "Hook - SYNT_S");
			return FALSE;
		}
		cmd::write(GRAY, "Hook - SYNT_S", CNetPtrMgr::sub_7FF7AA059CC4);

		//received_clone_create
		if (MH_CreateHook(CNetPtrMgr::received_clone_create, hk_received_clone_create, (void**)&og_received_clone_create) != MH_OK)
		{
			cmd::write(RED, "Hook - SYNT_C");
			return FALSE;
		}
		if (MH_EnableHook(CNetPtrMgr::received_clone_create) != MH_OK)
		{
			cmd::write(RED, "Hook - SYNT_C");
			return FALSE;
		}
		cmd::write(GRAY, "Hook - SYNT_C", CNetPtrMgr::received_clone_create);

		//received_clone_create_ack
		/*
		if (MH_CreateHook(CNetPtrMgr::received_clone_create_ack, hk_received_clone_create_ack, (void**)&og_received_clone_create_ack) != MH_OK)
		{
			cmd::write(RED, "Hook - SYNT_CA");
			return FALSE;
		}
		if (MH_EnableHook(CNetPtrMgr::received_clone_create_ack) != MH_OK)
		{
			cmd::write(RED, "Hook - SYNT_CA");
			return FALSE;
		}
		cmd::write(GRAY, "Hook - SYNT_CA", CNetPtrMgr::received_clone_create_ack);
		*/

		
		//ReadSyncTreeFromBuffer
		if ((location = mem::find(s_decrypt(patterns::ReadSyncTreeFromBuffer).c_str(), &srScanCxt).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - SYNT_A");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - SYNT_A", location);
		CNetPtrMgr::ReadSyncTreeFromBuffer = (fpReadSyncTreeFromBuffer)location;
		location = NULL;
		
		if (MH_CreateHook(CNetPtrMgr::ReadSyncTreeFromBuffer, hk_ReadSyncTreeFromBuffer, (void**)&og_ReadSyncTreeFromBuffer) != MH_OK)
		{
			cmd::write(RED, "Hook - SYNT_A");
			return FALSE;
		}
		if (MH_EnableHook(CNetPtrMgr::ReadSyncTreeFromBuffer) != MH_OK)
		{
			cmd::write(RED, "Hook - SYNT_A");
			return FALSE;
		}
		cmd::write(GRAY, "Hook - SYNT_A", CNetPtrMgr::ReadSyncTreeFromBuffer);
		
		//GetSyncTreeFromId
		if ((location = mem::find(s_decrypt(patterns::GetSyncTreeFromId).c_str(), &srScanCxt).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - SYNT_G");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - SYNT_G", location);
		CNetPtrMgr::GetSyncTreeFromId = (fpGetSyncTree)location;
		location = NULL;

		//vtCNetworkPlayerMgr
		if ((location = mem::find(s_decrypt(patterns::vtCNetworkPlayerMgr).c_str(), &srScanCxt).add(3)->rip()->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - NPMV");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - NPMV", location);
		CVtableMgr::vtCNetworkPlayerMgr = (__int64)location;
		CNetPtrMgr::RemovePlayerFromNetworkMgr = (fpRemovePlayerFromNetworkMgr)(*(__int64*)((__int64)location + (5 * 8)));
		location = NULL;

		return TRUE;
	}

	static BOOL FindNatives()
	{
		LPVOID location = NULL;

		mem::module_ctx scan_ctx;
		scan_ctx.start = (DWORD_PTR)gta5::baseAddress;
		scan_ctx.end = (DWORD_PTR)gta5::baseAddress + gta5::moduleSize;

		//does_cam_exists
		if ((location = mem::find(s_decrypt(patterns::does_cam_exists).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - DCE");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - DCE", location);
		CNativesMgr::does_cam_exists = (fpDoesCamExists)location;
		location = NULL;

		//get_player_name
		if ((location = mem::find(s_decrypt(patterns::get_player_name).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - GPN");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - GPN");
		CNativesMgr::get_player_name = (fpGetPlayerName)location;
		location = NULL;

		//player_id
		if ((location = mem::find(s_decrypt(patterns::player_id).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - PID");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - PID");
		CNativesMgr::player_id = (fpPlayerId)location;
		location = NULL;

		//draw_rect
		if ((location = mem::find(s_decrypt(patterns::draw_rect).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - DR");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - DR", location);
		CNativesMgr::draw_rect = (fpDrawRect)location;
		location = NULL;

		//begin_text_cmd_display_text
		if ((location = mem::find(s_decrypt(patterns::begin_text_cmd_display_text).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - GFX_1");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - GFX_1", location);
		CNativesMgr::begin_text_cmd_display_text = (fpBeginTextCmdDisplayText)location;
		location = NULL;

		//end_text_cmd_display_text
		if ((location = mem::find(s_decrypt(patterns::end_text_cmd_display_text).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - GFX_2");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - GFX_2", location);
		CNativesMgr::end_text_cmd_display_text = (fpEndTextCmdDisplayText)location;
		location = NULL;

		//add_text_comp_substring_playername
		if ((location = mem::find(s_decrypt(patterns::add_text_comp_substring_playername).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - GFX_3");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - GFX_3", location);
		CNativesMgr::add_text_component_substring_playername = (fpAddTextCompSubstrPlayerName)location;
		location = NULL;

		//get_hash_key
		if ((location = mem::find(s_decrypt(patterns::get_hash_key).c_str(), &scan_ctx).get_call()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - GHK");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - GHK", location);
		CNativesMgr::get_hash_key = (fpGetHashKey)location;
		location = NULL;

		//get_player_ped
		if ((location = mem::find(s_decrypt(patterns::get_player_ped).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - GPP");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - GPP ", location);
		CNativesMgr::get_player_ped = (fpGetPlayerPed)location;
		location = NULL;

		//play_sound_frontend
		if ((location = mem::find(s_decrypt(patterns::play_sound_frontend).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - PLSFE");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - PLSFE  ", location);
		CNativesMgr::play_sound_frontend = (fpPlaySoundFrontend)location;
		location = NULL;

		//add_explosion
		if ((location = mem::find(s_decrypt(patterns::add_explosion).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - AEX");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - AEX  ", location);
		CNativesMgr::add_explosion = (fpAddExplosion)location;
		location = NULL;

		//does_entity_exists
		if ((location = mem::find(s_decrypt(patterns::does_entity_exists).c_str(), &scan_ctx).add(7)->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - DEEX");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - DEEX", location);
		CNativesMgr::does_entity_exists = (fpDoesEntityExists)location;
		location = NULL;

		//get_vehicle_ped_is_using
		if ((location = mem::find(s_decrypt(patterns::get_vehicle_ped_is_using).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - GVPIU");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - GVPIU ", location);
		CNativesMgr::get_vehicle_ped_is_using = (fpGetVehiclePedIsUsing)location;
		location = NULL;

		//get_ped_is_in_vehicle_seat
		if ((location = mem::find(s_decrypt(patterns::get_ped_is_in_vehicle_seat).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - GPIIVS");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - GPIIVS ", location);
		CNativesMgr::get_ped_is_in_vehicle_seat = (fpGetPedIsInVehicleSeat)location;
		location = NULL;

		//clear_ped_task_immediately
		if ((location = mem::find(s_decrypt(patterns::clear_ped_task_immediately).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - CLPTI");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - CLPTI ", location);
		CNativesMgr::clear_ped_task_immediately = (fpClearPedTaskImmediately)location;
		location = NULL;

		//set_ped_into_vehicle
		if ((location = mem::find(s_decrypt(patterns::set_ped_into_vehicle).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - SPIV");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - SPIV", location);
		CNativesMgr::set_ped_into_vehicle = (fpSetPedIntoVehicle)location;
		location = NULL;

		//set_ped_ammo_by_type
		if ((location = mem::find(s_decrypt(patterns::set_ped_ammo_by_type).c_str(), &scan_ctx).add(2)->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - SPABT");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - SPABT ", location);
		CNativesMgr::set_ped_ammo_by_type = (fpSetPedAmmoByType)location;
		location = NULL;

		//network_is_game_in_progress
		if ((location = mem::find(s_decrypt(patterns::network_is_game_in_progress).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - IGIP");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - IGIP", location);
		CNativesMgr::network_is_game_in_progress = (fpNetworkIsGameInProgress)location;
		location = NULL;

		//network_has_control_of_entity
		if ((location = mem::find(s_decrypt(patterns::network_has_control_of_entity).c_str(), &scan_ctx).add(5)->get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - NHCOE");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - NHCOE ", location);
		CNativesMgr::network_has_control_of_entity = (fpNetworkHasControlOfEntity)location;
		location = NULL;

		//is_model_valid
		if ((location = mem::find(s_decrypt(patterns::is_model_valid).c_str(), &scan_ctx).get_call()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - IMV");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - IMV ", location);
		CMemoryMgr::pIsModelValid = (fpIsModelValid)location;
		location = NULL;

		//remove_weapon_from_ped
		if ((location = mem::find(s_decrypt(patterns::remove_weapon_from_ped).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - RWFP");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - RWFP ", location);
		CNativesMgr::remove_weapon_from_ped = (fpRemoveWeaponFromPed)location;
		location = NULL;

		//set_vehicle_fixed
		if ((location = mem::find(s_decrypt(patterns::set_vehicle_fixed).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - SVF");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - SVF", location);
		CNativesMgr::set_vehicle_fixed = (fpSetVehicleFixed)location;
		location = NULL;

		//get_host_of_script
		if ((location = mem::find(s_decrypt(patterns::get_host_of_script).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - GHOS");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - GHOS ", location);
		CNativesMgr::get_host_of_script = (fpGetHostOfScript)location;
		location = NULL;

		//network_set_in_spectator_mode
		if ((location = mem::find(s_decrypt(patterns::network_set_in_spectator_mode).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - NSISM");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - NSISM ", location);
		CNativesMgr::network_set_in_spectator_mode = (fpNetworkSetInSpectatorMode)location;
		location = NULL;

		//network_session_kick_player
		if ((location = mem::find(s_decrypt(patterns::network_session_kick_player).c_str(), &scan_ctx).get_addr()) == NULL)
		{
			cmd::write(DARKYELLOW, "Signature - NSKP");
			return FALSE;
		}
		cmd::write(GRAY, "Signature - NSKP ", location);
		CNativesMgr::network_session_kick_player = (fpNetworkSessionKickPlayer)location;
		location = NULL;

		return TRUE;
	}
};