#pragma once

#include "types.h"
#include <inttypes.h>
#include "classes.h"
#include "x64tools.h"

//natives
typedef LPCSTR(__cdecl* fpGetPlayerName)(Player player);
typedef Player(__cdecl* fpPlayerId)();
typedef Ped(__cdecl* fpGetPlayerPed)(Player player);
typedef Vehicle(__cdecl* fpGetVehiclePedIsUsing)(Ped ped);
typedef void(__cdecl* fpDrawRect)(float x, float y, float width, float height, int r, int g, int b, int a);
typedef void(__cdecl* fpDrawLine)(Vector3* pos1, Vector3* pos2, int r, int g, int b, int a);
typedef VOID(__cdecl* fpSetPedIntoVehicle)(Ped ped, Vehicle vehicle, int seatIndex);
typedef Ped(__cdecl* fpGetPedIsInVehicleSeat)(Vehicle vehicle, int seatIndex, BOOL p2);
typedef void(__cdecl* fpAddExplosion)(float x, float y, float z, int explosionType, float damageScale, BOOL isAudible, BOOL isInvisible, float cameraShake, BOOL noDamage);
typedef BOOL(__cdecl* fpAddTextCompSubstrPlayerName)(const char* text);
typedef BOOL(__cdecl* fpEndTextCmdDisplayText)(float x, float y, int unk1);
typedef BOOL(__cdecl* fpBeginTextCmdDisplayText)(const char* text);
typedef char(__cdecl* fpDoesCamExists)(Cam cam);
typedef Hash(__cdecl* fpGetHashKey)(LPCSTR string);
typedef VOID(__cdecl* fpPlaySoundFrontend)(int soundId, const char* audioName, const char* audioRef, BOOL p3);
typedef BYTE(__cdecl* fpDoesEntityExists)(Entity entity);
typedef VOID(__cdecl* fpClearPedTaskImmediately)(Ped ped);
typedef BYTE(__cdecl* fpNetworkIsGameInProgress)();
typedef VOID(__cdecl* fpSetPedAmmoByType)(Ped ped, Hash ammoTypeHash, int ammo);
typedef BOOL(__cdecl* fpNetworkHasControlOfEntity)(Entity entity);
typedef BOOL(__cdecl* fpIsModelValid)(__int64 model);
typedef VOID(__cdecl* fpRemoveWeaponFromPed)(Ped ped, Hash weaponHash);
typedef VOID(__cdecl* fpSetVehicleFixed)(Vehicle vehicle);
typedef Player(__cdecl* fpGetHostOfScript)(const char* scriptName, int p1, int p2);
typedef VOID(__cdecl* fpNetworkSetInSpectatorMode)(BOOL toggle, Ped ped);
typedef VOID(__cdecl* fpNetworkSessionKickPlayer)(Player player);

//network utils
typedef __int64(__cdecl* fpGetNetGamePlayerFromId)(unsigned __int8 player);
typedef char(__cdecl* fpReadBitbufferDWORD)(__int64 bitBuffer, unsigned int* data, int bits);
typedef char(__cdecl* fpReadBitbufferWORD)(__int64 bitbuffer, unsigned short* data, unsigned int len);
typedef char(__cdecl* fpMigrateScriptHost)(__int64 self, unsigned __int8* pCNetGamePlayer, unsigned __int16 a3, char a4);
typedef __int64(__cdecl* fpSendNetObjPlane)(__int64 a1, unsigned __int16 a2, char a3, __int16 a4, __int16 a5, unsigned int a6);
typedef __int64(__cdecl* fpSub7FF7AA059CC4)(__int64* netObject);
typedef __int64(__cdecl* fpGetNetPlayerData)(__int64 pCNetGamePlayer);
typedef __int64*(__cdecl* fpGetPedClass)(unsigned int entity);
typedef __int64(__cdecl* fpGetEntityFromClass)(__int64 pCEntity);
typedef void(__cdecl* fpRemovePlayerFromNetworkMgr)(__int64 pCNetworkPlayerMgr, __int64 pCNetGamePlayer);

//network events
typedef BOOL(__cdecl* fpExplosionEvent)(__int64 netGameEvent, __int64 sender);
typedef __int64(__cdecl* fpKickvotesEvent)(__int64 netGameEvent, __int64 bitbuffer, __int64 sender);
typedef char(__cdecl* fpRequestControlEvent)(__int64 netGameEvent, __int64 sender, __int64 receiver);
typedef char(__cdecl* fpScriptedEvent)(__int64 netGameEvent, __int64 sender);
typedef char(__cdecl* fpRemoveWeaponEvent)(__int64 netGameEvent, __int64 bitbuffer, __int64 sender);
typedef void(__cdecl* fpTriggerScriptEvent)(int event_type, unsigned int* event_data, int len, int playerbits);
typedef char(__cdecl* fpClearPedTaskEvent)(__int64 netGameEvent, __int64 sender);
typedef char(__cdecl* fpIncrementStatEvent)(__int64 netGameEvent, __int64 sender);
typedef char(__cdecl* fpInfoChangeEvent)(unsigned long* netGameEvent, __int64 sender);
typedef char(__cdecl* fpCrcHashCheckEvent)(__int64 netGameEvent, __int64 sender);
typedef char(__cdecl* fpCheckExeSizeEvent)(__int64 netGameEvent, __int64 sender);

//sync trees
typedef __int64(__cdecl* fpReceivedCloneCreate)(__int64 pNetworkObjectMgr, __int64 sender, __int64 receiver,
	unsigned __int16 sync_tree_id, unsigned __int16 a5, unsigned __int16 sync_tree_type, __int64 a7, unsigned int timestamp);

typedef __int64(__cdecl* fpReceivedCloneCreateAck)(__int64 pNetworkObjectMgr, __int64 sender, __int64 receiver, unsigned __int16 a4, unsigned int ackCode);

typedef __int64(__cdecl* fpReceivedCloneSync)(__int64 pNetworkObjectMgr, __int64 sender, __int64
	receiver, unsigned __int16 sync_tree_id, unsigned __int16 a5, __int64 a6, unsigned __int16 a7, unsigned int timestamp);

typedef __int64(__cdecl* fpReadSyncTreeFromBuffer)(unsigned __int64** sync_tree_ptr, __int64 a2, unsigned int a3, __int64 a4, __int64 a5);
typedef __int64(__cdecl* fpGetSyncTree)(__int64 self, unsigned __int16 sync_tree_id);

class CMemoryMgr
{
public:

	//pointers
	static inline uint32_t* tick_count = NULL;
	static inline __int64* pCNetworkPlayerMgr = NULL;

	static inline fpReadBitbufferDWORD read_bitbuffer_dword = NULL;
	static inline fpReadBitbufferWORD read_bitbuffer_word = NULL;
	static inline fpGetNetGamePlayerFromId pGetNetPlayerFromId = NULL;
	static inline fpGetPedClass pGetPedClassFromId = NULL;
	static inline fpGetEntityFromClass pGetEntityFromClass = NULL;
	static inline fpIsModelValid pIsModelValid = NULL;

	static inline char ReadBitbufferDWORD(__int64 bitbuffer, uint32_t* data, int bits)
	{
		return x64::spoof_call(read_bitbuffer_dword, &x64::fastcall_ctx, bitbuffer, data, bits);
	}

	static inline __int64 GetNetGamePlayerFromId(unsigned __int8 player)
	{
		return x64::spoof_call(pGetNetPlayerFromId, &x64::fastcall_ctx, player);
	}

	static inline __int64 GetNetPlayerData(Player player)
	{
		__int64 pCNetGamePlayer = GetNetGamePlayerFromId(player);
		if (pCNetGamePlayer)
		{
			__int64 vTable = *(__int64*)pCNetGamePlayer;
			fpGetNetPlayerData getPlayerData = (fpGetNetPlayerData)(*(__int64*)(vTable + 0x30));
			return x64::spoof_call(getPlayerData, &x64::fastcall_ctx, pCNetGamePlayer);
		}
		return 0;
	}

	static inline __int64* GetPedClassFromId(unsigned int player)
	{
		return x64::spoof_call(pGetPedClassFromId, &x64::fastcall_ctx, player);
	}

	static inline __int64 GetEntityFromClass(__int64 pCEntity)
	{
		return x64::spoof_call(pGetEntityFromClass, &x64::fastcall_ctx, pCEntity);
	}

	static inline BOOL IsModelValid(__int64 model)
	{
		return x64::spoof_call(pIsModelValid, &x64::fastcall_ctx, model);
	}
};

class CVtableMgr
{
public:

	static inline __int64 vtCGameScriptHandlerNetComponent = NULL;
	static inline __int64 vtCNetworkObjectMgr = NULL;
	static inline __int64 vtCNetworkPlayerMgr = NULL;
	static inline __int64 vtCPlane = NULL;
};

class CNetPtrMgr
{
public:

	//network events
	static inline fpExplosionEvent explosion_event = NULL;
	static inline fpKickvotesEvent kickvotes_event = NULL;
	static inline fpRequestControlEvent request_control_event = NULL;
	static inline fpScriptedEvent scripted_event = NULL;
	static inline fpIncrementStatEvent increment_stat_event = NULL;
	static inline fpRemoveWeaponEvent remove_weapon_event = NULL;
	static inline fpTriggerScriptEvent trigger_script_event = NULL;
	static inline fpClearPedTaskEvent clear_ped_task_event = NULL;
	static inline fpInfoChangeEvent info_change_event = NULL;
	static inline fpCrcHashCheckEvent crc_hash_check_event = NULL;
	static inline fpCheckExeSizeEvent check_exe_size_event = NULL;

	//network utils
	static inline fpMigrateScriptHost MigrateScriptHost = NULL;
	static inline fpSendNetObjPlane SendNetObjPlane = NULL;
	static inline fpSub7FF7AA059CC4 sub_7FF7AA059CC4 = NULL;
	static inline fpRemovePlayerFromNetworkMgr RemovePlayerFromNetworkMgr = NULL;

	//sync trees
	static inline fpReceivedCloneCreate received_clone_create = NULL;
	static inline fpReceivedCloneCreateAck received_clone_create_ack = NULL;
	static inline fpReceivedCloneSync received_clone_sync = NULL;
	static inline fpReadSyncTreeFromBuffer ReadSyncTreeFromBuffer = NULL;
	static inline fpGetSyncTree GetSyncTreeFromId = NULL;
};

class CNativesMgr
{
public:

	static inline fpDoesCamExists does_cam_exists = NULL;
	static inline fpGetPlayerName get_player_name = NULL;
	static inline fpDrawLine draw_line = NULL;
	static inline fpDrawRect draw_rect = NULL;
	static inline fpAddTextCompSubstrPlayerName add_text_component_substring_playername = NULL;
	static inline fpBeginTextCmdDisplayText begin_text_cmd_display_text = NULL;
	static inline fpEndTextCmdDisplayText end_text_cmd_display_text = NULL;
	static inline fpGetHashKey get_hash_key = NULL;
	static inline fpGetPedIsInVehicleSeat get_ped_is_in_vehicle_seat = NULL;
	static inline fpGetVehiclePedIsUsing get_vehicle_ped_is_using = NULL;
	static inline fpGetPlayerPed get_player_ped = NULL;
	static inline fpSetPedAmmoByType set_ped_ammo_by_type = NULL;
	static inline fpNetworkHasControlOfEntity network_has_control_of_entity = NULL;
	static inline fpGetHostOfScript get_host_of_script = NULL;


	static inline fpPlayerId player_id = NULL; //invoked
	static inline fpPlaySoundFrontend play_sound_frontend = NULL; //invoked
	static inline fpAddExplosion add_explosion = NULL; //invoked
	static inline fpDoesEntityExists does_entity_exists = NULL; //invoked
	static inline fpClearPedTaskImmediately clear_ped_task_immediately = NULL; //invoked
	static inline fpSetPedIntoVehicle set_ped_into_vehicle = NULL; //invoked
	static inline fpNetworkIsGameInProgress network_is_game_in_progress = NULL; //invoked
	static inline fpRemoveWeaponFromPed remove_weapon_from_ped = NULL; //invoked
	static inline fpSetVehicleFixed set_vehicle_fixed = NULL; //invoked
	static inline fpNetworkSetInSpectatorMode network_set_in_spectator_mode = NULL; //invoked
	static inline fpNetworkSessionKickPlayer network_session_kick_player = NULL; //invoked
};