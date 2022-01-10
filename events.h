#pragma once

#include "game.h"
#include "x64tools.h"
#include "log.h"
#include "hud.h"
#include "invoker.h"
#include "network.h"

/*

sync trees

- vehicle hash
	__int64 pCVehicleCreationDataNode = *(__int64*)((unsigned __int64)pNetSyncTree + 48);
	Hash mHash = *(Hash*)(pCVehicleCreationDataNode + 200);

*/

/*

	Bounty = 1294995624,
			CeoBan = -764524031,
			CeoKick = 248967238,
			CeoMoney = 1890277845,
			ClearWantedLevel = -91354030,
			FakeDeposit = 677240627,
			ForceMission = 2020588206,
			GtaBanner = 1572255940,
			PersonalVehicleDestroyed = 802133775,
			RemoteOffradar = -391633760,
			RotateCam = 801199324,
			SendToCutscene = 1068259786,
			SendToIsland = -621279188,
			SoundSpam = 1132878564,
			Spectate = -1113591308,
			Teleport = 603406648,
			TransactionError = -1704141512,
			VehicleKick = 578856274,
			RemoteOTR = 1722873242

*/

class CMitigationMgr
{
public:

	//nt events mitigations
	static inline BOOL bExplosionProt = FALSE;
	static inline BOOL bRmWeaponsProt = TRUE;
	static inline BOOL bClearTasksProt = TRUE;
	static inline BOOL bRequestCtrlProt = FALSE;

	//crash mitigation
	static inline BOOL bSyncCrashProt = TRUE;

	static inline BOOL bBlockScriptedEvents = TRUE;
	static inline BOOL bBlockFriendlyScriptedEvents = FALSE;

	static inline BOOL bLogScriptEvents = FALSE;
	static inline BOOL bLogSyncTree = FALSE;
	static inline BOOL bSpawnCrashObjects = FALSE;
};

static Hash lastScriptEventHashReceived = 0;

static fpExplosionEvent og_explosion_event = NULL;
static BOOL __cdecl hk_explosion_event(__int64 netGameEvent, __int64 sender)
{
	Player player_id = *(__int8*)(sender + 33);
	CHudMgr::notify(ENColor::NYELLOW, "Explosion event", "from: %s", x64::spoof_call(CNativesMgr::get_player_name, &x64::fastcall_ctx, player_id));
	cmd::write(YELLOW, "Explosion event, from: %s", x64::spoof_call(CNativesMgr::get_player_name, &x64::fastcall_ctx, player_id));

	if (CMitigationMgr::bExplosionProt)
		return true;

	return x64::spoof_call(og_explosion_event, &x64::fastcall_ctx, netGameEvent, sender);
}

static void NotifyBreakFreemodeKick(LPCSTR sender_name, Player player)
{
	SPlayer* psPlayer = CPlayerListMgr::getPlayer(player);
	if (psPlayer)
	{
		if (!psPlayer->srFlags.isModding)
		{
			CHudMgr::notify(ENColor::NYELLOW, "Break freemode", "from: %s", sender_name);
			cmd::write(YELLOW, "Break freemode, from %s", sender_name);
			psPlayer->srFlags.isModding = TRUE;
			CPlayerListMgr::RefreshPlayerAttributes(player);
		}
	}
}

static void NotifyScriptCrash(LPCSTR sender_name, Player player)
{
	SPlayer* psPlayer = CPlayerListMgr::getPlayer(player);
	if (psPlayer)
	{
		if (!psPlayer->srFlags.isModding)
		{
			CHudMgr::notify(ENColor::NYELLOW, "Crash attempt", "from: %s", sender_name);
			cmd::write(YELLOW, "Crash attempt, from %s", sender_name);
			psPlayer->srFlags.isModding = TRUE;
			CPlayerListMgr::RefreshPlayerAttributes(player);
		}
	}
}

static void NotifyScriptCrashEx(LPCSTR sender_name, Player player, Hash hash)
{
	SPlayer* psPlayer = CPlayerListMgr::getPlayer(player);
	if (psPlayer)
	{
		if (!psPlayer->srFlags.isModding)
		{
			CHudMgr::notify(ENColor::NYELLOW, "Crash attempt (ex)", "from: %s", sender_name);
			cmd::write(YELLOW, "Crash attempt (ex), from %s, hash: %d", sender_name, hash);
			psPlayer->srFlags.isModding = TRUE;
			CPlayerListMgr::RefreshPlayerAttributes(player);
		}
	}
}

static void NotifySpoofedScriptedEvent(LPCSTR sender_name, Player player)
{
	SPlayer* psPlayer = CPlayerListMgr::getPlayer(player);
	if (psPlayer)
	{
		if (!psPlayer->srFlags.isModding)
		{
			CHudMgr::notify(ENColor::NYELLOW, "Spoofed script event", "from: %s", sender_name);
			cmd::write(YELLOW, "Spoofed script event, from %s", sender_name);
			psPlayer->srFlags.isModding = TRUE;
			CPlayerListMgr::RefreshPlayerAttributes(player);
		}
	}
}

static fpScriptedEvent og_scripted_event = NULL;
static char __cdecl hk_scripted_event(__int64 netGameEvent, __int64 sender)
{
	auto len = *(uint32_t*)(netGameEvent + 548);
	auto data = (__int64*)(netGameEvent + 112);
	Player sender_id = *(__int8*)(sender + 33);
	LPCSTR sender_name = x64::spoof_call(CNativesMgr::get_player_name, &x64::fastcall_ctx, sender_id);

	lastScriptEventHashReceived = (Hash)data[0];

	if (CMitigationMgr::bLogScriptEvents)
		cmd::write(GRAY, "scripted event, from: %s, hash: %d", sender_name, data[0]);

	//hashes with no handler in freemode.c
	static Hash freemodeEventsWithNoHandler[] =
	{
		-1013679841, -1386010354, -1787741241, 962740265,
		-143116395, -488349268, -1872092614, -317318371, 296518236, 911179316,
		665075435, 125492983, -10584126, -1766066400, -786546101, -1367466623,
		-1026787486, -1402943861, -980869764, -522517025, -980869764,
		194348342, -1672632969, 637990725, -39307858, 1915516637, -292927152,
		-1321808667, 193360559, 1474930020, 1216755327, 1213478059, -1208585385,
		1649958326, 1649958326, -1896924387, 1119864805, -2041535807, -978812630, //
		-1857108105, -1500178943, -660428494, 1474930020, -586564007
	};

	if (CMitigationMgr::bBlockScriptedEvents)
	{

		if ((uint32_t)data[1] >= 32)
		{
			NotifyBreakFreemodeKick(sender_name, sender_id);
			return true;
		}

		if ((uint32_t)data[1] != sender_id)
		{
			NotifySpoofedScriptedEvent(sender_name, sender_id);
			return true;
		}

		for (int i = 0; i < sizeof(freemodeEventsWithNoHandler); i++)
		{
			if ((Hash)data[0] == freemodeEventsWithNoHandler[i])
			{
				NotifyScriptCrashEx(sender_name, sender_id, data[0]);
				return true;
			}
		}

		switch ((Hash)data[0])
		{
		case 1228916411: //network bail
			CHudMgr::notify(ENColor::NYELLOW, "Network bail", "from: %s", sender_name);
			cmd::write(YELLOW, "Network bail event, from %s", sender_name);
			return true;

		case 603406648: //apartment invite
			CHudMgr::notify(ENColor::NYELLOW, "Apartment invite", "from: %s", sender_name);
			cmd::write(YELLOW, "Apartment invite, from %s", sender_name);
			return true;
		
		case 248967238: //ceo kick
			CHudMgr::notify(ENColor::NYELLOW, "CEO kick", "from: %s", sender_name);
			cmd::write(YELLOW, "CEO kick, from %s", sender_name);

			if(CMitigationMgr::bBlockFriendlyScriptedEvents)
				return true;
			break;
		
		case -764524031: //ceo ban
			CHudMgr::notify(ENColor::NYELLOW, "CEO ban", "from: %s", sender_name);
			cmd::write(YELLOW, "CEO ban, from %s", sender_name);
			return true;
		
		case -621279188: //send to island
			CHudMgr::notify(ENColor::NYELLOW, "Send to island", "from: %s", sender_name);
			cmd::write(YELLOW, "Send to island, from %s", sender_name);
			return true;
		
		case -1704141512: //Transaction error
			CHudMgr::notify(ENColor::NYELLOW, "Transaction error", "from: %s", sender_name);
			cmd::write(YELLOW, "Transaction error, from %s", sender_name);
			return true;

		case 801199324:
			if ((int)data[2] == 537560473)
			{
				NotifyBreakFreemodeKick(sender_name, sender_id);
				return true;
			}
			break;

		case -145306724:
			if ((uint32_t)data[2] >= 32)
			{
				NotifyBreakFreemodeKick(sender_name, sender_id);
				return true;
			}
			break;

		case -581037897:
			if ((uint32_t)data[2] >= 62 || (uint32_t)data[3] >= 32)
			{
				NotifyBreakFreemodeKick(sender_name, sender_id);
				return true;
			}
			break;

		case 1757755807:
			if ((uint32_t)data[2] >= 62)
			{
				NotifyBreakFreemodeKick(sender_name, sender_id);
				return true;
			}
			break;

		case 436475575:
		case 990606644:
			if ((uint32_t)data[2] >= 20)
			{
				NotifyBreakFreemodeKick(sender_name, sender_id);
				return true;
			}
			break;

		default:
			break;
		}
	}
	return x64::spoof_call(og_scripted_event, &x64::fastcall_ctx, netGameEvent, sender);
}

static fpRemoveWeaponEvent og_remove_weapon_event = NULL;
static char __cdecl hk_remove_weapon_event(__int64 netGameEvent, __int64 bitbuffer, __int64 sender)
{
	Player sender_id = *(__int8*)(sender + 33);
	LPCSTR sender_name = x64::spoof_call(CNativesMgr::get_player_name, &x64::fastcall_ctx, sender_id);
	unsigned short wHash = 255;

	x64::spoof_call(CMemoryMgr::read_bitbuffer_word, &x64::fastcall_ctx, bitbuffer, &wHash, (unsigned int)13);
	*(unsigned long*)(bitbuffer + 16) = 0; // reset bitbuffer cursor

	if (wHash == NULL)
	{
		CHudMgr::notify(ENColor::NYELLOW, "Crash attempt (rm)", "from: %s", sender_name);
		cmd::write(RED, "Crash attempt (rm), from: %s", sender_name);
		return true;
	}

	CHudMgr::notify(ENColor::NYELLOW, "Remove weapon", "from: %s", sender_name);

	SPlayer* psPlayer = CPlayerListMgr::getPlayer(sender_id);
	if (psPlayer)
	{
		if (!psPlayer->srFlags.isModding)
		{
			psPlayer->srFlags.isModding = TRUE;
			CPlayerListMgr::RefreshPlayerAttributes(sender_id);
		}
	}

	if (CMitigationMgr::bRmWeaponsProt)
		return true;

	return x64::spoof_call(og_remove_weapon_event, &x64::fastcall_ctx, netGameEvent, bitbuffer, sender);
}

static fpClearPedTaskEvent og_clear_ped_task_event = NULL;
static char __cdecl hk_clear_ped_task_event(__int64 netGameEvent, __int64 sender)
{
	Player sender_id = *(__int8*)(sender + 33);
	LPCSTR sender_name = x64::spoof_call(CNativesMgr::get_player_name, &x64::fastcall_ctx, sender_id);
	CHudMgr::notify(ENColor::NYELLOW, "Clear tasks", "from: %s", sender_name);

	if (CMitigationMgr::bClearTasksProt)
		return TRUE;

	return x64::spoof_call(og_clear_ped_task_event, &x64::fastcall_ctx, netGameEvent, sender);
}

static Player lastSyncTreeSender = 0;
static Hash lastSyncModelHashReceived = 0;
static fpReceivedCloneCreate og_received_clone_create = NULL;
static __int64 __cdecl hk_received_clone_create(__int64 pCNetworkObjectMgr, __int64 sender, __int64 receiver,
	unsigned __int16 sync_tree_id, unsigned __int16 a5, unsigned __int16 sync_tree_type, __int64 a7, unsigned int timestamp)
{

	if (CMitigationMgr::bSyncCrashProt)
	{
		Player sender_id = *(__int8*)(sender + 33);
		lastSyncTreeSender = sender_id;
		SPlayer* psSender = CPlayerListMgr::getPlayer(sender_id);
		if (psSender)
		{
			if (psSender->srFlags.bBlockSyncs)
				return true;
		}

		//invalid sync tree
		if (sync_tree_id > 13)
		{
			LPCSTR sender_name = x64::spoof_call(CNativesMgr::get_player_name, &x64::fastcall_ctx, sender_id);
			CHudMgr::notify(NYELLOW, "Invalid sync", "from: %s", sender_name);
			cmd::write(YELLOW, "Invalid sync, from: %s", sender_name);

			SPlayer* psPlayer = CPlayerListMgr::getPlayer(sender_id);
			if (psPlayer)
			{
				if (!psPlayer->srFlags.isModding)
				{
					psPlayer->srFlags.isModding = TRUE;
					CPlayerListMgr::RefreshPlayerAttributes(sender_id);
				}
			}
			return true;
		}
	}
	return x64::spoof_call(og_received_clone_create, &x64::fastcall_ctx, pCNetworkObjectMgr, sender, receiver, sync_tree_id, a5, sync_tree_type, a7, timestamp);
}

static fpReceivedCloneCreateAck og_received_clone_create_ack = NULL;
static __int64 __cdecl hk_received_clone_create_ack(__int64 pNetworkObjectMgr, __int64 sender, __int64 receiver, unsigned __int16 a4, unsigned int ackCode)
{
	// todo stuff
	return x64::spoof_call(og_received_clone_create_ack, &x64::fastcall_ctx, pNetworkObjectMgr, sender, receiver, a4, ackCode);
}

static fpReceivedCloneSync og_received_clone_sync = NULL;
static __int64 __cdecl hk_received_clone_sync(__int64 pCNetworkObjectMgr, __int64 sender, __int64 receiver,
	unsigned __int16 sync_tree_id, unsigned __int16 a5, __int64 a6, unsigned __int16 a7, unsigned int timestamp)
{
	if (CMitigationMgr::bSyncCrashProt)
	{
		Player sender_id = *(__int8*)(sender + 33);
		lastSyncTreeSender = sender_id;
		SPlayer* psSender = CPlayerListMgr::getPlayer(sender_id);
		if (psSender)
		{
			if (psSender->srFlags.bBlockSyncs)
				return true;
		}

		//invalid sync tree
		if (sync_tree_id > 13)
		{
			LPCSTR sender_name = x64::spoof_call(CNativesMgr::get_player_name, &x64::fastcall_ctx, sender_id);
			CHudMgr::notify(NYELLOW, "Invalid sync", "from: %s", sender_name);
			cmd::write(YELLOW, "Invalid sync, from: %s", sender_name);

			SPlayer* psPlayer = CPlayerListMgr::getPlayer(sender_id);
			if (psPlayer)
			{
				if (!psPlayer->srFlags.isModding)
				{
					psPlayer->srFlags.isModding = TRUE;
					CPlayerListMgr::RefreshPlayerAttributes(sender_id);
				}
			}
			return true;
		}
	}
	return x64::spoof_call(og_received_clone_sync, &x64::fastcall_ctx, pCNetworkObjectMgr, sender, receiver, sync_tree_id, a5, a6, a7, timestamp);
}

static inline __int64 GetSyncTreeFromId(unsigned __int16 id)
{
	return x64::spoof_call(CNetPtrMgr::GetSyncTreeFromId, &x64::fastcall_ctx, (__int64)NULL, (unsigned __int16)id);
}

static inline Hash joaat(LPCSTR string)
{
	return x64::spoof_call(CNativesMgr::get_hash_key, &x64::fastcall_ctx, string);
}

static fpReadSyncTreeFromBuffer og_ReadSyncTreeFromBuffer = NULL;
static __int64 __cdecl hk_ReadSyncTreeFromBuffer(unsigned __int64** pNetSyncTree, __int64 a2, unsigned int a3, __int64 a4, __int64 a5)
{
	auto ret = x64::spoof_call(og_ReadSyncTreeFromBuffer, &x64::fastcall_ctx, pNetSyncTree, a2, a3, a4, a5);

	if (CMitigationMgr::bSyncCrashProt)
	{
		Hash mHash = 0;
		__int64 pCEntityCreationDataNode = NULL;

		for (int i = 1; i < 13; i++)
		{
			if (GetSyncTreeFromId(i) == (__int64)pNetSyncTree)
			{
				LPCSTR sender_name = x64::spoof_call(CNativesMgr::get_player_name, &x64::fastcall_ctx, lastSyncTreeSender);
				switch (i) // i = syncTreeId
				{
				case 5:  // CObjectSyncTree
					if (CMitigationMgr::bLogSyncTree)
						cmd::write(GRAY, "CObjectSyncTree: %p", pNetSyncTree);

					pCEntityCreationDataNode = *(__int64*)((unsigned __int64)pNetSyncTree + 48);
					mHash = *(Hash*)(pCEntityCreationDataNode + 336);

					if (!CMemoryMgr::IsModelValid(mHash))
					{
						*(Hash*)(pCEntityCreationDataNode + 336) = joaat("prop_paints_can07");
						cmd::write(YELLOW, "Invalid sync model, from: %s", sender_name);
						CHudMgr::notify(NYELLOW, "Invalid sync model", "from: %s", sender_name);
					}
					break;

					switch (mHash)
					{
					case 379820688: // prop_dog_cage_01
					case 1692612370: // prop_dog_cage_02
					case 959275690: // prop_gold_cont_01
					case 1396140175: // prop_gold_cont_01b

						cmd::write(YELLOW, "Cage spawn, from: %s", sender_name);
						CHudMgr::notify(NYELLOW, "Cage spawn", "from: %s", sender_name);

						SPlayer* psPlayer = CPlayerListMgr::getPlayer(lastSyncTreeSender);
						if (psPlayer)
						{
							psPlayer->srFlags.isModding = TRUE;
							CPlayerListMgr::RefreshPlayerAttributes(lastSyncTreeSender);
						}
						break;
						*(Hash*)(pCEntityCreationDataNode + 336) = joaat("prop_paints_can07");
					}

					break;

				case 11:  // CPlayerSyncTree
					//if (CMitigationMgr::bLogSyncTree)
						//cmd::write(GRAY, "CPlayerSyncTree: %p", pNetSyncTree);
					break;

				case 0:  // CAutomobileSyncTree
				case 1:	 // CBikeSyncTree
				case 2:	 // CBoatSyncTree
				case 4:  // CHeliSyncTree
				case 9:  // CPlaneSyncTree
				case 12: // CAutomobileSyncTree
				case 13: // CTrainSyncTree

					//if (CMitigationMgr::bLogSyncTree)
						//cmd::write(GRAY, "CVehicleSyncTree: %p", pNetSyncTree);

					pCEntityCreationDataNode = *(__int64*)((unsigned __int64)pNetSyncTree + 48);
					mHash = *(Hash*)(pCEntityCreationDataNode + 200);
					lastSyncModelHashReceived = mHash;

					//patch the vehicle hash if its invalid
					if (!CMemoryMgr::IsModelValid(mHash))
					{
						LPCSTR sender_name = x64::spoof_call(CNativesMgr::get_player_name, &x64::fastcall_ctx, lastSyncTreeSender);
						*(Hash*)(pCEntityCreationDataNode + 200) = joaat("comet");
						cmd::write(YELLOW, "Invalid sync model, from: %s", sender_name);
						CHudMgr::notify(NYELLOW, "Invalid sync model", "from: %s", sender_name);
					}
					break;
				}
				break;
			}
		}
	}
	return ret;
}

static fpIncrementStatEvent og_increment_stat_event = NULL;
static char __cdecl hk_increment_stat_event(__int64 netGameEvent, __int64 sender)
{
	Player sender_id = *(__int8*)(sender + 33);
	LPCSTR sender_name = x64::spoof_call(CNativesMgr::get_player_name, &x64::fastcall_ctx, sender_id);
	Hash sHash = *(unsigned long*)(netGameEvent + 0x30);

	if (sHash == x64::spoof_call(CNativesMgr::get_hash_key, &x64::fastcall_ctx, "mpply_exploits")
		|| sHash == x64::spoof_call(CNativesMgr::get_hash_key, &x64::fastcall_ctx, "mpply_vc_annoyingme")
		|| sHash == x64::spoof_call(CNativesMgr::get_hash_key, &x64::fastcall_ctx, "mpply_tc_annoyingme")
		|| sHash == x64::spoof_call(CNativesMgr::get_hash_key, &x64::fastcall_ctx, "mpply_vc_hate")
		|| sHash == x64::spoof_call(CNativesMgr::get_hash_key, &x64::fastcall_ctx, "mpply_tc_hate")
		|| sHash == x64::spoof_call(CNativesMgr::get_hash_key, &x64::fastcall_ctx, "mpply_bad_crew_name")
		|| sHash == x64::spoof_call(CNativesMgr::get_hash_key, &x64::fastcall_ctx, "mpply_bad_crew_motto")
		|| sHash == x64::spoof_call(CNativesMgr::get_hash_key, &x64::fastcall_ctx, "mpply_bad_crew_status")
		|| sHash == x64::spoof_call(CNativesMgr::get_hash_key, &x64::fastcall_ctx, "mpply_bad_crew_emblem")
		|| sHash == x64::spoof_call(CNativesMgr::get_hash_key, &x64::fastcall_ctx, "mpply_playermade_title")
		|| sHash == x64::spoof_call(CNativesMgr::get_hash_key, &x64::fastcall_ctx, "mpply_playermade_desc")
		|| sHash == x64::spoof_call(CNativesMgr::get_hash_key, &x64::fastcall_ctx, "mpply_griefing")
		|| sHash == x64::spoof_call(CNativesMgr::get_hash_key, &x64::fastcall_ctx, "mpply_game_exploits"))
	{
		CHudMgr::notify(NYELLOW, "Report event", "from: %s", sender_name);
		cmd::write(YELLOW, "Report event, from: %s", sender_name);

		SPlayer* psPlayer = CPlayerListMgr::getPlayer(sender_id);
		if (psPlayer)
		{
			if (!psPlayer->srFlags.hasReported)
			{
				psPlayer->srFlags.hasReported = TRUE;
				CPlayerListMgr::RefreshPlayerAttributes(sender_id);
			}
		}

		return true;
	}

	CHudMgr::notify(NYELLOW, "Increment stat event", "from: %s", sender_name);
	cmd::write(YELLOW, "Increment stat event, from: %s", sender_name);
	return x64::spoof_call(og_increment_stat_event, &x64::fastcall_ctx, netGameEvent, sender);
}

static fpRequestControlEvent og_request_control_event = NULL;
static char __cdecl hk_request_control_event(__int64 netGameEvent, __int64 sender, __int64 receiver)
{
	Player sender_id = *(__int8*)(sender + 33);
	LPCSTR sender_name = x64::spoof_call(CNativesMgr::get_player_name, &x64::fastcall_ctx, sender_id);

	CHudMgr::notify(NPURPLE, "Request control", "from: %s", sender_name);
	if (CMitigationMgr::bRequestCtrlProt)
		return true;
	return x64::spoof_call(og_request_control_event, &x64::fastcall_ctx, netGameEvent, sender, receiver);
}

static fpInfoChangeEvent og_info_change_event = NULL;
static char __cdecl hk_info_change_event(unsigned long* netGameEvent, __int64 sender)
{
	char buff[64];
	Player sender_id = *(__int8*)(sender + 33);

	if (sender_id == invoke<Player>(CNativesMgr::player_id))
		return true;

	LPCSTR sender_name = x64::spoof_call(CNativesMgr::get_player_name, &x64::fastcall_ctx, sender_id);
	if (strcmp(sender_name, "**Invalid**") == 0)
		return x64::spoof_call(og_info_change_event, &x64::fastcall_ctx, netGameEvent, sender);

	SPlayer* psPlayer = CPlayerListMgr::getPlayer(sender_id);
	if (psPlayer)
	{
		if (!psPlayer->srFlags.isModding)
		{
			CHudMgr::notify(NCYAN, "Info change event", "from: %s", sender_name);
			cmd::write(RED, "%s is modding! (info change)", sender_name);
			psPlayer->srFlags.isModding = TRUE;
			CPlayerListMgr::RefreshPlayerAttributes(sender_id);
		}
	}
	return x64::spoof_call(og_info_change_event, &x64::fastcall_ctx, netGameEvent, sender);
}

static fpCrcHashCheckEvent og_crc_hash_check_event = NULL;
static char __cdecl hk_crc_hash_check_event(__int64 netGameEvent, __int64 sender)
{
	char buff[64];
	Player sender_id = *(__int8*)(sender + 33);

	if (sender_id == invoke<Player>(CNativesMgr::player_id))
		return true;

	LPCSTR sender_name = x64::spoof_call(CNativesMgr::get_player_name, &x64::fastcall_ctx, sender_id);
	if (strcmp(sender_name, "**Invalid**") == 0)
		return x64::spoof_call(og_crc_hash_check_event, &x64::fastcall_ctx, netGameEvent, sender);

	SPlayer* psPlayer = CPlayerListMgr::getPlayer(sender_id);
	if (psPlayer)
	{
		psPlayer->srFlags.isExeCorrupted = TRUE;
		CPlayerListMgr::RefreshPlayerAttributes(sender_id);
	}
	return x64::spoof_call(og_crc_hash_check_event, &x64::fastcall_ctx, netGameEvent, sender);
}

static fpCheckExeSizeEvent og_check_exe_size_event = NULL;
static char __cdecl hk_check_exe_size_event(__int64 netGameEvent, __int64 sender)
{
	char buff[64];
	Player sender_id = *(__int8*)(sender + 33);

	if (sender_id == invoke<Player>(CNativesMgr::player_id))
		return true;

	LPCSTR sender_name = x64::spoof_call(CNativesMgr::get_player_name, &x64::fastcall_ctx, sender_id);
	if (strcmp(sender_name, "**Invalid**") == 0)
		return x64::spoof_call(og_check_exe_size_event, &x64::fastcall_ctx, netGameEvent, sender);

	SPlayer* psPlayer = CPlayerListMgr::getPlayer(sender_id);
	if (psPlayer)
	{
		psPlayer->srFlags.isExeCorrupted = TRUE;
		CPlayerListMgr::RefreshPlayerAttributes(sender_id);
	}
	return x64::spoof_call(og_check_exe_size_event, &x64::fastcall_ctx, netGameEvent, sender);
}

static fpMigrateScriptHost og_migrate_script_host = NULL;
static unsigned __int16 lastToken = 0;
static __int64 pCGameScriptHandlerNetComponent = NULL;

static char __cdecl hk_migrate_script_host(__int64 pCGameScriptHandlerNetComponenet, unsigned __int8* pCNetGamePlayer, unsigned __int16 a3, char a4)
{
	if (pCGameScriptHandlerNetComponent == NULL)
		pCGameScriptHandlerNetComponent = pCGameScriptHandlerNetComponenet;

	if (a3 > lastToken)
		lastToken = a3;

	//cmd::write(GRAY, "migrate_script_host, netPlayer: %p, a3: %d, a4: %d", pCNetGamePlayer, a3, a4);
	return x64::spoof_call(og_migrate_script_host, &x64::fastcall_ctx, pCGameScriptHandlerNetComponenet, pCNetGamePlayer, a3, a4);
}

static fpSendNetObjPlane og_SendNetObjPlane = NULL;
static __int64 __cdecl hk_SendNetObjPlane(__int64 pCPlane, unsigned __int16 a2, char ownerId, __int16 a4, __int16 a5, unsigned int a6)
{
	cmd::write(GRAY, "SendNetObjPlane, a1: %p, a2: %d, a3: %d", pCPlane, a2, ownerId);
	cmd::write(GRAY, "a4: %p, a5: %p, a6: %d", a4, a5, a6);
	return x64::spoof_call(og_SendNetObjPlane, &x64::fastcall_ctx, pCPlane, a2, ownerId, a4, a5, a6);
}

static fpSub7FF7AA059CC4 og_sub_7FF7AA059CC4 = NULL;
static __int64 __cdecl hk_sub_7FF7AA059CC4(__int64* netObject)
{
	if (CMitigationMgr::bSpawnCrashObjects)
		*(unsigned __int16*)((__int64)netObject + 8) = 14;

	return x64::spoof_call(og_sub_7FF7AA059CC4, &x64::fastcall_ctx, netObject);
}