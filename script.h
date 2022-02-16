#pragma once

#include <vector>
#include "x64tools.h"
#include "game.h"
#include "log.h"
#include "config.h"
#include "invoker.h"
#include "hud.h"
#include "events.h"
#include "network.h"

typedef bool(__cdecl* fpIsHost)(__int64 self);

class script
{

public:

	static VOID setup()
	{
		CreateMenu();

		CFrontend::isFrontendActive = TRUE;
		CFrontend::isMenuActive = FALSE;

		CGraphicsMgr::fMenuX = WND_DEFAULT_OFFSET_X;
		CGraphicsMgr::fMenuY = WND_DEFAULT_OFFSET_Y;
	}

	static VOID tick()
	{
		//refresh playerlist
		LPCSTR sCurrentPlayerName = NULL;
		CHAR pBuffMsg[64];
		CMenu* pMenuPlayerlist = CFrontend::getMenuByName(MENU_PLAYERLIST);

		//add and notify incoming players
		Ped currRemotePed;
		SPlayer* pCurrentNetPlayer = NULL;
		
		/* bad method to refresh playerlist... */
		for (DWORD p = 0; p < 32; p++)
		{
			currRemotePed = get_player_ped(p);
			pCurrentNetPlayer = CPlayerListMgr::getPlayer(p);
			sCurrentPlayerName = x64::spoof_call(CNativesMgr::get_player_name, &x64::fastcall_ctx, (int)p);

			if (does_entity_exists(currRemotePed))
			{
				if (!CPlayerListMgr::IsPlayerInList(sCurrentPlayerName))
				{
					if (pMenuPlayerlist)
					{
						CNetworkPlayerMenu* menu_network_player = NULL;
						if ((menu_network_player = reinterpret_cast<CNetworkPlayerMenu*>(CFrontend::getMenuByName("menu_networked_player"))) != NULL)
						{
							pMenuPlayerlist->AddTrigger(sCurrentPlayerName, sCurrentPlayerName, NULL, 1)->AttachMenu(menu_network_player);
							memset(pBuffMsg, 0, sizeof(pBuffMsg));
							sprintf(pBuffMsg, MSG_ON_PLAYER_JOIN_NTF, sCurrentPlayerName);
							cmd::write(CYAN, MSG_ON_PLAYER_JOIN_CMD, sCurrentPlayerName);
							CHudMgr::notify(ENColor::NGREEN, pBuffMsg);
							CPlayerListMgr::AddPlayerToList(sCurrentPlayerName, p);
						}
					}
					continue;
				}
				else
				{
					if (pCurrentNetPlayer)
					{
						if (strcmp(pCurrentNetPlayer->sPlayerName, sCurrentPlayerName) != 0)
						{
							CMenuEntry* entryToRemove = pMenuPlayerlist->getEntryByName(pCurrentNetPlayer->sPlayerName);
							pMenuPlayerlist->RemoveEntry(entryToRemove);
							memset(pBuffMsg, 0, sizeof(pBuffMsg));
							sprintf(pBuffMsg, MSG_ON_PLAYER_LEAVE_NTF, pCurrentNetPlayer->sPlayerName);
							CPlayerListMgr::RemovePlayerFromList(pCurrentNetPlayer->iPlayerIndex);
							continue;
						}

						if (network_is_game_in_progress())
						{
							/* get script host */
							if (pCurrentNetPlayer->iPlayerIndex == get_host_of_script("freemode", -1, 0))
							{
								if (!pCurrentNetPlayer->srFlags.scHost)
								{
									pCurrentNetPlayer->srFlags.scHost = TRUE;
									CPlayerListMgr::RefreshPlayerAttributes(pCurrentNetPlayer->iPlayerIndex);
								}
							}
							else
							{
								if (pCurrentNetPlayer->srFlags.scHost)
								{
									pCurrentNetPlayer->srFlags.scHost = FALSE;
									CPlayerListMgr::RefreshPlayerAttributes(pCurrentNetPlayer->iPlayerIndex);
								}
							}

							/* get session host */
							__int64 pCNetGamePlayer = CMemoryMgr::GetNetGamePlayerFromId(pCurrentNetPlayer->iPlayerIndex);
							if (pCNetGamePlayer)
							{
								__int64 vTable = *(__int64*)pCNetGamePlayer;
								fpIsHost isHost = (fpIsHost)(*(__int64*)(vTable + 0x28));
								SPlayer* currHost = NULL;

								if (x64::spoof_call(isHost, &x64::fastcall_ctx, pCNetGamePlayer))
								{
									if ((currHost = CPlayerListMgr::getHost()) != NULL)
									{
										if (currHost->iPlayerIndex != pCurrentNetPlayer->iPlayerIndex)
										{
											currHost->srFlags.isHost = FALSE;
											pCurrentNetPlayer->srFlags.isHost = TRUE;
											CPlayerListMgr::RefreshPlayerAttributes(currHost->iPlayerIndex);
											CPlayerListMgr::RefreshPlayerAttributes(pCurrentNetPlayer->iPlayerIndex);
										}
									}
									else
									{
										pCurrentNetPlayer->srFlags.isHost = TRUE;
										CPlayerListMgr::RefreshPlayerAttributes(pCurrentNetPlayer->iPlayerIndex);
									}
								}

							}
						}
					}
				}
			}
			else
			{
				if (pCurrentNetPlayer)
				{
					if (pMenuPlayerlist)
					{
						//check if the entry's submenu is open before deleting it
						CMenuEntry* entryToRemove = pMenuPlayerlist->getEntryByName(pCurrentNetPlayer->sPlayerName);
						if (entryToRemove)
							if (CFrontend::getTopMenu() == entryToRemove->get_submenu())
								CFrontend::PopMenu();
						pMenuPlayerlist->RemoveEntry(entryToRemove);

						memset(pBuffMsg, 0, sizeof(pBuffMsg));
						sprintf(pBuffMsg, MSG_ON_PLAYER_LEAVE_NTF, pCurrentNetPlayer->sPlayerName);
						cmd::write(CYAN, MSG_ON_PLAYER_LEAVE_CMD, pCurrentNetPlayer->sPlayerName);
						CHudMgr::notify(ENColor::NRED, pBuffMsg);
						CPlayerListMgr::RemovePlayerFromList(pCurrentNetPlayer->iPlayerIndex);
					}
				}
			}
		}

		if (!network_is_game_in_progress())
		{
			lastToken = 0;
			pCGameScriptHandlerNetComponent = NULL;
		}

		//refresh the hud
		CHudMgr::refresh();

		//inputs
		if ((get_key_just_pressed(VK_F4)))
		{
			if (CFrontend::isMenuActive)
			{
				CFrontend::isMenuActive = FALSE;
				play_sound_frontend(0, "CANCEL", "HUD_FRONTEND_DEFAULT_SOUNDSET", 0);
			}
			else
			{
				CFrontend::isMenuActive = TRUE;
				play_sound_frontend(0, "SELECT", "HUD_FRONTEND_DEFAULT_SOUNDSET", 0);

			}
		}

		if (get_key_just_pressed(VK_NUMPAD0))
		{
			if (CFrontend::isMenuActive)
			{
				if (CFrontend::getTopMenu() != CFrontend::getMenuByName("main"))
					CFrontend::getTopMenu()->ResetIndex();
				CFrontend::PopMenu();
				play_sound_frontend(0, "CANCEL", "HUD_FRONTEND_DEFAULT_SOUNDSET", 0);
			}
		}

		if (get_key_just_pressed(VK_NUMPAD5))
		{
			if (CFrontend::isMenuActive)
			{
				CMenu* pActiveMenu = CFrontend::getTopMenu();
				if (pActiveMenu)
				{
					CMenuEntry* pEntry = pActiveMenu->getEntryAtIndex(pActiveMenu->getIndex());
					if (pEntry)
					{
						CMenu* pAttachedMenu = NULL;
						if ((pAttachedMenu = pEntry->get_submenu()) != NULL)
						{
							CFrontend::PushMenu(pAttachedMenu);
							if (strcmp(pActiveMenu->getName(), MENU_PLAYERLIST) == 0)
							{
								INT playerIndex = CPlayerListMgr::getPlayerIdByName(pEntry->getName());
								if (playerIndex != -1)
								{
									CMenuEntry* pSpectateEntry = static_cast<CNetworkPlayerMenu*>(pAttachedMenu)->getEntryByName("entry_networked_player_spectate");
									if (pSpectateEntry)
									{
										SPlayer* psPlayer = CPlayerListMgr::getPlayer(playerIndex);
										if (psPlayer)
											pSpectateEntry->setStatusPtr(&psPlayer->srFlags.isSpectated);
									}
									CMenuEntry* pBlockSyncsEntry = static_cast<CNetworkPlayerMenu*>(pAttachedMenu)->getEntryByName("entry_networked_player_block_syncs");
									if (pBlockSyncsEntry)
									{
										SPlayer* psPlayer = CPlayerListMgr::getPlayer(playerIndex);
										if (psPlayer)
											pBlockSyncsEntry->setStatusPtr(&psPlayer->srFlags.bBlockSyncs);
									}

									static_cast<CNetworkPlayerMenu*>(pAttachedMenu)->setPlayerId(playerIndex); /* submenu is menu_networked_player */
									pAttachedMenu->setDisplayName(pEntry->getName());
								}
							}

							if (pActiveMenu->getType() == eMenuType::PLAYER)
							{
								if (pAttachedMenu->getType() == eMenuType::PLAYER)
								{
									INT playerIndex = static_cast<CNetworkPlayerMenu*>(pActiveMenu)->getPlayerId();
									static_cast<CNetworkPlayerMenu*>(pAttachedMenu)->setPlayerId(playerIndex);
									static_cast<CNetworkPlayerMenu*>(pAttachedMenu)->TriggerOnOpenCallback();
								}
							}
						}
						else
						{
							pActiveMenu = CFrontend::getTopMenu();
							if (pActiveMenu)
								pActiveMenu->TriggerEntry();
						}
						play_sound_frontend(0, "SELECT", "HUD_FRONTEND_DEFAULT_SOUNDSET", 0);
					}
				}
			}
		}

		if (get_key_just_pressed(VK_NUMPAD8))
		{
			if (CFrontend::isMenuActive)
			{
				CFrontend::LowerMenuIndex();
				play_sound_frontend(0, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", 0);
			}
		}

		if (get_key_just_pressed(VK_NUMPAD2))
		{
			if (CFrontend::isMenuActive)
			{
				CFrontend::RaiseMenuIndex();
				play_sound_frontend(0, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", 0);
			}
		}

		if (get_key_just_pressed(VK_NUMPAD3))
		{
			Ped ped = get_player_ped(player_id());
			if (does_entity_exists(ped))
			{
				Vector3 pos = get_player_coords(ped);
				add_explosion(pos.x, pos.y, pos.z, eExplosionTag::EXP_TAG_GRENADE, 1.f, !bStealthExplosions, bStealthExplosions, FALSE, FALSE);
			}
		}

		if (get_key_just_pressed(VK_NUMPAD6))
		{
			if (CFrontend::isMenuActive)
			{
				CMenu* activeMenu = CFrontend::getTopMenu();
				CMenuEntry* entry = activeMenu->getEntryAtIndex(activeMenu->getIndex());

				if (activeMenu && entry)
				{
					INT prev_index;
					if (entry->get_type() == eEntryType::SLIDER)
					{
						prev_index = reinterpret_cast<CSliderEntry*>(entry)->getIndex();
						prev_index++;
						reinterpret_cast<CSliderEntry*>(entry)->setIndex(prev_index);
					}
					if (entry->get_type() == eEntryType::PLAYER_SLIDER_TRIGGER)
					{
						prev_index = reinterpret_cast<CPlayerSliderEntry*>(entry)->getIndex();
						prev_index++;
						reinterpret_cast<CPlayerSliderEntry*>(entry)->setIndex(prev_index);
					}
					if (entry->get_type() == eEntryType::PLAYER_SLIDER_TOGGLE)
					{
						prev_index = reinterpret_cast<CSliderToggleEntry*>(entry)->getIndex();
						prev_index++;
						reinterpret_cast<CSliderToggleEntry*>(entry)->setIndex(prev_index);
					}
					play_sound_frontend(0, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", 0);
				}
			}
		}

		if (get_key_just_pressed(VK_NUMPAD4))
		{
			if (CFrontend::isMenuActive)
			{
				CMenu* pActiveMenu = CFrontend::getTopMenu();
				CMenuEntry* pEntry = pActiveMenu->getEntryAtIndex(pActiveMenu->getIndex());
				eEntryType type = pEntry->get_type();

				if (pActiveMenu && pEntry)
				{
					INT prevIndex;
					if (type == eEntryType::SLIDER)
					{
						prevIndex = reinterpret_cast<CSliderEntry*>(pEntry)->getIndex();
						prevIndex--;
						reinterpret_cast<CSliderEntry*>(pEntry)->setIndex(prevIndex);
					}
					if (type == eEntryType::PLAYER_SLIDER_TRIGGER)
					{
						prevIndex = reinterpret_cast<CPlayerSliderEntry*>(pEntry)->getIndex();
						prevIndex--;
						reinterpret_cast<CPlayerSliderEntry*>(pEntry)->setIndex(prevIndex);
					}
					if (type == eEntryType::PLAYER_SLIDER_TOGGLE)
					{
						prevIndex = reinterpret_cast<CSliderToggleEntry*>(pEntry)->getIndex();
						prevIndex--;
						reinterpret_cast<CSliderToggleEntry*>(pEntry)->setIndex(prevIndex);
					}
					play_sound_frontend(0, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", 0);
				}
			}
		}

	}

private:

	static inline Ped get_player_ped(Player player)
	{
		return invoke<Ped>(CNativesMgr::get_player_ped, player);
	}

	static inline Ped player_ped_id()
	{
		return get_player_ped(player_id());
	}

	static inline BYTE does_entity_exists(Entity entity)
	{
		return x64::spoof_call<BYTE>(CNativesMgr::does_entity_exists, &x64::fastcall_ctx, entity);
	}

	static inline VOID remove_weapon_from_ped(Ped ped, Hash weaponHash)
	{
		return x64::spoof_call(CNativesMgr::remove_weapon_from_ped, &x64::fastcall_ctx, ped, weaponHash);
	}

	static inline Ped get_ped_is_in_vehicle_seat(Vehicle vehicle, INT seatIndex, BOOL p2)
	{
		return x64::spoof_call(CNativesMgr::get_ped_is_in_vehicle_seat, &x64::fastcall_ctx, vehicle, seatIndex, p2);
	}

	static inline LPCSTR get_player_name(Player player)
	{
		return x64::spoof_call(CNativesMgr::get_player_name, &x64::fastcall_ctx, player);
	}

	static inline Player player_id()
	{
		return invoke<Player>(CNativesMgr::player_id);
	}

	static inline VOID play_sound_frontend(INT soundId, LPCSTR audioName, LPCSTR audioRef, BOOL p3)
	{
		invoke<Void>(CNativesMgr::play_sound_frontend, soundId, audioName, audioRef, p3);
	}

	static inline Player get_host_of_script(const char* scriptName, int p1, int p2)
	{
		return x64::spoof_call(CNativesMgr::get_host_of_script, &x64::fastcall_ctx, scriptName, p1, p2);
	}

	static inline VOID set_ped_ammo_by_type(Ped ped, Hash ammoTypeHash, INT ammo)
	{
		x64::spoof_call(CNativesMgr::set_ped_ammo_by_type, &x64::fastcall_ctx, ped, ammoTypeHash, ammo);
	}


	static inline VOID add_explosion(FLOAT x, FLOAT y, FLOAT z, INT explosionType, FLOAT damageScale, BOOL isAudible, BOOL isInvisible, FLOAT cameraShake, BOOL noDamage)
	{
		invoke<Void>(CNativesMgr::add_explosion, x, y, z, explosionType, damageScale, isAudible, isInvisible, cameraShake, noDamage);
	}

	static inline Vehicle get_vehicle_ped_is_using(Ped ped)
	{
		//return x64::spoof_call(CNativesMgr::get_vehicle_ped_is_using, &x64::fastcall_ctx, ped);
		return invoke<Vehicle>(CNativesMgr::get_vehicle_ped_is_using, ped);
	}

	static inline BYTE network_is_game_in_progress()
	{
		return invoke<BYTE>(CNativesMgr::network_is_game_in_progress);
	}

	static inline VOID network_set_in_spectator_mode(BOOL toggle, Ped ped)
	{
		x64::spoof_call(CNativesMgr::network_set_in_spectator_mode, &x64::fastcall_ctx, toggle, ped);
		//invoke<Void>(CNativesMgr::network_set_in_spectator_mode, toggle, ped);
	}

	static inline VOID network_session_kick_player(Player player)
	{
		invoke<Void>(CNativesMgr::network_session_kick_player, player);
	}

	static inline BOOL network_has_control_of_entity(Entity entity)
	{
		return x64::spoof_call(CNativesMgr::network_has_control_of_entity, &x64::fastcall_ctx, entity);
	}

	static inline Vector3 get_player_coords(Entity entity)
	{
		Vector3 coords{};
		__int64* pEntityClass = CMemoryMgr::GetPedClassFromId(entity);
		if (pEntityClass)
		{
			coords.x = *(float*)((DWORD_PTR)pEntityClass + 0x90);
			coords.y = *(float*)((DWORD_PTR)pEntityClass + 0x94);
			coords.z = *(float*)((DWORD_PTR)pEntityClass + 0x98);
		}
		return coords;
	}

	static BOOL get_key_just_pressed(INT key)
	{
		static SHORT last_key = 0;

		if (last_key != 0)
		{
			if (last_key != key)
				return FALSE;
			else
			{
				//reset input on key up
				if (x64::spoof_call(GetAsyncKeyState, &x64::fastcall_ctx, key))
					return FALSE;
				else
					last_key = 0;
			}
		}
		else
		{
			if (x64::spoof_call(GetAsyncKeyState, &x64::fastcall_ctx, VK_LSHIFT & VK_RSHIFT))
				return FALSE;

			if (x64::spoof_call(GetAsyncKeyState, &x64::fastcall_ctx, key))
			{
				last_key = key;
				return TRUE;
			}
		}
		return FALSE;
	}

	static inline Hash joaat(LPCSTR text)
	{
		return x64::spoof_call(CNativesMgr::get_hash_key, &x64::fastcall_ctx, text);
	}

	static inline VOID clear_ped_task_immediately(Ped ped)
	{
		x64::spoof_call(CNativesMgr::clear_ped_task_immediately, &x64::fastcall_ctx, ped);
	}

	static inline VOID set_vehicle_fixed(Vehicle vehicle)
	{
		x64::spoof_call(CNativesMgr::set_vehicle_fixed, &x64::fastcall_ctx, vehicle);
	}

	static inline VOID set_ped_into_vehicle(Ped ped, Vehicle vehicle, INT seatIndex)
	{
		invoke<Void>(CNativesMgr::set_ped_into_vehicle, ped, vehicle, seatIndex);
	}

	/*----------------------ui callbacks------------------------*/

	static inline VOID handlerExplodePlayer(CPlayerSliderEntry* slider, Player player)
	{
		if (slider)
		{
			Ped ped = get_player_ped(player);
			if (does_entity_exists(ped))
			{
				Vector3 pos = get_player_coords(ped);
				if (pos.x != 0 && pos.y != 0 && pos.z != 0)
					add_explosion(pos.x, pos.y, pos.z, slider->get_current_value()->iValue, 1.f, !bStealthExplosions, bStealthExplosions, FALSE, FALSE);
			}
		}
	}

	static VOID handlerMaxAmmo(CMenuEntry* entry)
	{
		Ped pPed = player_ped_id();
		if (does_entity_exists(pPed))
		{
			for (DWORD i = 0; i < sizeof(AmmoTypes) / 4; i++)
				set_ped_ammo_by_type(pPed, AmmoTypes[i], 9999);
		}
	}

	static VOID handlerClearConsole(CMenuEntry* entry)
	{
		system("cls");
	}

	static VOID handlerKickFromVehicle(CMenuEntry* entry, Player player)
	{
		Ped pPed = get_player_ped(player);
		if (does_entity_exists(pPed))
		{
			Vehicle veh = get_vehicle_ped_is_using(pPed);
			if (does_entity_exists(veh))
				clear_ped_task_immediately(pPed);
		}
	}

	static VOID handlerSyncCrashLobby(CMenuEntry* entry)
	{
		if (network_is_game_in_progress())
			CMitigationMgr::bSpawnCrashObjects = TRUE;
	}

	static VOID handlerCrashPlayer(CMenuEntry* entry, Player player)
	{
		if (network_is_game_in_progress())
		{
			if (player == player_id())
			{
				CHudMgr::notify(ENColor::NYELLOW, "Are you crazy?");
				return;
			}

			Ped pPed = get_player_ped(player);
			if (does_entity_exists(pPed))
			{
				uint64_t data[] = { -1386010354, player_id(), 4212424, 442214, 0, 0, 0, 0, 0, 0 };
				x64::spoof_call(CNetPtrMgr::trigger_script_event, &x64::fastcall_ctx, 1, (uint32_t*)data, 10, (1 << player));
			}
		}
	}

	static VOID handleHostKick(CMenuEntry* entry, Player player)
	{
		if (network_is_game_in_progress())
		{
			__int64 pCNetGamePlayer = CMemoryMgr::GetNetGamePlayerFromId(player_id());
			if (pCNetGamePlayer)
			{
				__int64 vTable = *(__int64*)pCNetGamePlayer;
				fpIsHost isHost = (fpIsHost)(*(__int64*)(vTable + 0x28));
				if (x64::spoof_call(isHost, &x64::fastcall_ctx, pCNetGamePlayer))
				{
					Ped pPed = get_player_ped(player);
					if (does_entity_exists(pPed))
						network_session_kick_player(player);
				}
			}
		}
	}

	static VOID handlerMigrateScriptHost(CMenuEntry* entry)
	{
		if (network_is_game_in_progress())
		{
			if (get_host_of_script("freemode", -1, 0) != player_id())
			{
				if (pCGameScriptHandlerNetComponent != NULL && lastToken != 0)
				{
					unsigned __int8* pCNetGamePlayer = (unsigned __int8*)CMemoryMgr::GetNetGamePlayerFromId(player_id());
					if (pCNetGamePlayer)
					{
						cmd::write(MAGENTA, "Attempting script host migration...");
						CHudMgr::notify(NPURPLE, "Attempting script host", "migration...");
						x64::spoof_call(og_migrate_script_host, &x64::fastcall_ctx, pCGameScriptHandlerNetComponent, pCNetGamePlayer, ++lastToken, (char)1);
					}
				}
			}
		}
	}

	static VOID handlerDesyncKick(CMenuEntry* entry, Player player)
	{
		if (network_is_game_in_progress())
		{
			if (player == player_id() || CPlayerListMgr::getHost()->iPlayerIndex == player)
				return;

			__int64 pCNetGamePlayer = CMemoryMgr::GetNetGamePlayerFromId(player);
			if(pCNetGamePlayer)
				x64::spoof_call(CNetPtrMgr::RemovePlayerFromNetworkMgr, &x64::fastcall_ctx, *(__int64*)CMemoryMgr::pCNetworkPlayerMgr, pCNetGamePlayer);
		}
	}

	static VOID handlerBreakFreemodeKick(CMenuEntry* entry, Player player)
	{
		/* { hash, size } */
		static Hash break_freemode_hashes[][2] =
		{
			{-581037897, 4},
			{-1208585385, 5},
			{-1948352103, 5},
			{1213478059, 8},
			{-1767058336, 3},
			{-1782442696, 4},
			{-586564007, 7},
			{-890540460, 3}
		};

		if (network_is_game_in_progress())
		{
			if (player == player_id())
			{
				CHudMgr::notify(ENColor::NYELLOW, "Are you crazy?");
				return;
			}

			Ped pPed = get_player_ped(player);
			if (does_entity_exists(pPed))
			{
				uint64_t data[] = { 0, 32, 90909, 909090, 90909, 90909, 90909, 0, 0, 0, 0, 0, 0 };
				int len = 0;

				for (int i = 0; i < (sizeof(break_freemode_hashes) / sizeof(break_freemode_hashes[0])); i++)
				{
					data[0] = break_freemode_hashes[i][0];
					len = break_freemode_hashes[i][1];

					if (does_entity_exists(pPed))
						x64::spoof_call(CNetPtrMgr::trigger_script_event, &x64::fastcall_ctx, 1, (uint32_t*)data, len, (1 << player));
					else { break; }
				}
			}
		}
	}

	static VOID handlerSendToIsland(CMenuEntry* entry, Player player)
	{
		if (network_is_game_in_progress())
		{
			Ped pPed = get_player_ped(player);
			if (does_entity_exists(pPed))
			{
				uint64_t data[] = { -621279188, CPlayerListMgr::getHost()->iPlayerIndex, 1, 0, 1 };
				x64::spoof_call(CNetPtrMgr::trigger_script_event, &x64::fastcall_ctx, 1, (uint32_t*)data, 5, (1 << player));
			}
		}
	}

	static VOID handlerCeoBan(CMenuEntry* entry, Player player)
	{
		if (network_is_game_in_progress())
		{
			Ped pPed = get_player_ped(player);
			if (does_entity_exists(pPed))
			{
				uint64_t data[] = { -764524031, CPlayerListMgr::getHost()->iPlayerIndex, 1 };
				x64::spoof_call(CNetPtrMgr::trigger_script_event, &x64::fastcall_ctx, 1, (uint32_t*)data, 3, (1 << player));
			}
		}
	}

	static VOID handlerCeoKick(CMenuEntry* entry, Player player)
	{
		if (network_is_game_in_progress())
		{
			Ped pPed = get_player_ped(player);
			if (does_entity_exists(pPed))
			{
				uint64_t data[] = { 248967238, CPlayerListMgr::getHost()->iPlayerIndex, 1, 5 };
				x64::spoof_call(CNetPtrMgr::trigger_script_event, &x64::fastcall_ctx, 1, (uint32_t*)data, 4, (1 << player));
			}
		}
	}

	static VOID handlerBlockSyncs(CMenuEntry* entry, Player player)
	{
		if (network_is_game_in_progress())
		{
			Ped pPed = get_player_ped(player);
			if (does_entity_exists(pPed))
			{
				SPlayer* psPlayer = NULL;
				if ((psPlayer = CPlayerListMgr::getPlayer(player)) != NULL)
					psPlayer->srFlags.bBlockSyncs = !psPlayer->srFlags.bBlockSyncs;
			}
		}
	}

	static VOID handlerSpectatePlayer(CMenuEntry* entry, Player player)
	{
		if (network_is_game_in_progress())
		{
			Ped pPed = get_player_ped(player);
			if (does_entity_exists(pPed))
			{
				SPlayer* psPlayer = NULL;
				SPlayer* specPsPlayer = NULL;

				for (DWORD i = 0; i < 32; i++)
				{
					psPlayer = CPlayerListMgr::getPlayer(i);
					if (psPlayer)
					{
						if (psPlayer->srFlags.isSpectated && psPlayer->iPlayerIndex != player)
							return;
						if (psPlayer->iPlayerIndex == player)
							specPsPlayer = psPlayer;
					}
				}

				if (specPsPlayer)
				{
					specPsPlayer->srFlags.isSpectated = !specPsPlayer->srFlags.isSpectated;
					network_set_in_spectator_mode(specPsPlayer->srFlags.isSpectated, pPed);
					CPlayerListMgr::RefreshPlayerAttributes(specPsPlayer->iPlayerIndex);
				}
			}
		}
	}

	static VOID handlerRepairVehicle(CMenuEntry* entry)
	{
		Vehicle veh = get_vehicle_ped_is_using(player_ped_id());
		if (does_entity_exists(veh))
		{
			if (network_is_game_in_progress())
			{
				if (!network_has_control_of_entity(veh))
					return;
			}
			set_vehicle_fixed(veh);
		}
	}

	static VOID handlerTeleportInsideVehicle(CMenuEntry* entry, Player player)
	{
		if (network_is_game_in_progress())
		{
			Ped pPed = get_player_ped(player);
			if (does_entity_exists(pPed))
			{
				Vehicle pVehicle = get_vehicle_ped_is_using(pPed);
				if (does_entity_exists(pVehicle))
					set_ped_into_vehicle(player_ped_id(), pVehicle, -2);
			}
		}
	}

	static VOID handlerBoolPointer(CMenuEntry* entry)
	{
		if (entry->get_type() == TOGGLE)
		{
			BOOL* status = entry->getStatusPtr();
			*status = !*status;
		}
	}

	static Hash* GetWeaponHashArray(eWeaponGroup id)
	{
		Hash* ret = NULL;
		switch (id)
		{
		case wpPISTOL:
			ret = wp_pistols;
			break;
		case wpMG:
			ret = wp_mgs;
			break;
		case wpRIFLE:
			ret = wp_rifles;
			break;
		case wpSNIPER:
			ret = wp_snipers;
			break;
		case wpMELEE:
			ret = wp_melees;
			break;
		case wpSHOTGUN:
			ret = wp_shotguns;
			break;
		case wpHEAVY:
			ret = wp_heavys;
			break;
		case wpSPECIAL:
			ret = wp_specials;
			break;
		default:
			break;
		}
		return ret;
	}

	static VOID handlerRemoveWeaponGroup(CPlayerSliderEntry* entry, Player player)
	{
		Ped pPed = get_player_ped(player);
		if (does_entity_exists(pPed))
		{
			Hash* wp_group = GetWeaponHashArray((eWeaponGroup)entry->get_current_value()->iValue);
			if (wp_group)
			{
				while (*wp_group)
				{
					remove_weapon_from_ped(pPed, *wp_group);
					wp_group = (Hash*)((uint64_t)wp_group + sizeof(Hash));
				}
			}
		}
	}

	static VOID handlerRemoveAllWeapons(CMenuEntry* entry, Player player)
	{
		Ped pPed = get_player_ped(player);
		if (does_entity_exists(pPed))
		{
			Hash* wp_group = NULL;
			for (int wp_gid = 0;; wp_gid++)
			{
				if ((wp_group = GetWeaponHashArray((eWeaponGroup)wp_gid)) == NULL)
					break;

				while (*wp_group)
				{
					remove_weapon_from_ped(pPed, *wp_group);
					wp_group = (Hash*)((uint64_t)wp_group + sizeof(Hash));
				}
			}
		}
	}

	static VOID handlerPrintCNetGamePlayer(CMenuEntry* entry, Player player)
	{
		Ped pPed = get_player_ped(player);
		if (does_entity_exists(pPed))
			cmd::write(WHITE, "CNetGamePlayer: %p", CMemoryMgr::GetNetGamePlayerFromId(player));
	}

	static VOID handlerPrintNetPlayerData(CMenuEntry* entry, Player player)
	{
		if (network_is_game_in_progress())
		{
			Ped pPed = get_player_ped(player);
			if (does_entity_exists(pPed))
				cmd::write(WHITE, "netPlayerData: %p", CMemoryMgr::GetNetPlayerData(player));
		}
	}

	static VOID handlerPrintPedClass(CMenuEntry* entry, Player player)
	{
		Ped pPed = get_player_ped(player);
		if (does_entity_exists(pPed))
			cmd::write(WHITE, "CPed: %p", CMemoryMgr::pGetPedClassFromId(pPed));
	}

	static VOID OnOpenMenuPlayerInfo(CNetworkPlayerMenu* thisptr, Player id)
	{
		CMenuEntry* entry = NULL;
		CTextEntry* t_entry = NULL;
		__int64 netData = CMemoryMgr::GetNetPlayerData(id);

		char buf[32];
		struct in_addr ip_addr;

		if (netData == NULL)
		{
			CHudMgr::notify(NRED, "Error: could not", "retrieve player data!");
			return;
		}

		memset(buf, 0, sizeof(buf));
		for (INT i = 0; i < thisptr->getEntriesNum(); i++)
		{
			entry = thisptr->getEntryAtIndex(i);
			if (entry)
			{
				if (entry->get_type() == eEntryType::TEXT)
				{
					t_entry = static_cast<CTextEntry*>(entry);

					switch (i)
					{
					case 0:
						t_entry->setLabel(get_player_name(id));
						break;
					case 1:
						ip_addr.S_un.S_addr = *(unsigned __int32*)(netData + 0x4C);
						t_entry->setLabel(inet_ntoa(ip_addr));
						break;

					case 2:
						sprintf_s(buf, "%llu", *(__int64*)(netData + 0x08));
						t_entry->setLabel(buf);
						memset(buf, 0, sizeof(buf));
						break;

					case 3:
						sprintf_s(buf, "%lu", *(unsigned __int32*)(netData + 0x60));
						t_entry->setLabel(buf);
						memset(buf, 0, sizeof(buf));
						break;

					case 4:
						sprintf_s(buf, "%d", id);
						t_entry->setLabel(buf);
						memset(buf, 0, sizeof(buf));
						break;

					default:
						break;
					}
				}
			}
		}
	}

	script() {};
	~script() {};

public:

	static inline BOOL bHasSetupRun = FALSE;

	//feats
	//static inline BOOL bNeverWanted = FALSE;
	static inline BOOL bStealthExplosions = FALSE;
	static inline BOOL bIsInSpectatorMode = FALSE;

private:

	static VOID CreateMenu()
	{
		//create all menus here

		//player menu
		CMenu* pMenuSelf = new CMenu("menu_player", "Player", WND_DEFAULT_HEADER_COLOR);
		//pMenuSelf->AddButton("entry_player_neverwanted", "Never wanted", handlerBoolPointer, 1, NULL, &bNeverWanted);
		CFrontend::RegisterMenu(pMenuSelf);

		//-----------------------network menus-----------------------------

		//playerlist menu
		CMenu* pMenuPlayerlist = new CMenu(MENU_PLAYERLIST, "Players", WND_DEFAULT_HEADER_COLOR);
		CFrontend::RegisterMenu(pMenuPlayerlist);

		//network events protections
		CMenu* pNetEventsProtections = new CMenu("menu_prot_network_events",
			"Network Events", WND_DEFAULT_HEADER_COLOR);
		pNetEventsProtections->AddButton("entry_prot_network_events_explosions",
			"Explosions", handlerBoolPointer, 1, NULL, &CMitigationMgr::bExplosionProt);

		pNetEventsProtections->AddButton("entry_prot_network_events_rm_weapons",
			"Remove weapons", handlerBoolPointer, 1, NULL, &CMitigationMgr::bRmWeaponsProt);

		pNetEventsProtections->AddButton("entry_prot_network_events_clear_tasks",
			"Clear tasks", handlerBoolPointer, 1, NULL, &CMitigationMgr::bClearTasksProt);

		pNetEventsProtections->AddButton("entry_prot_network_events_request_ctrl",
			"Request control", handlerBoolPointer, 1, NULL, &CMitigationMgr::bRequestCtrlProt);

		CFrontend::RegisterMenu(pNetEventsProtections);

		//protections menu
		CMenu* pMenuProtections = new CMenu("menu_prots", "Protections", WND_DEFAULT_HEADER_COLOR);
		pMenuProtections->AddTrigger("entry_prots_network_events", "Network events", NULL, 1)->AttachMenu(pNetEventsProtections);
		pMenuProtections->AddButton("entry_prots_scripts_events", "Script events", handlerBoolPointer, 1, NULL, &CMitigationMgr::bBlockScriptedEvents);
		pMenuProtections->AddButton("entry_prots_scripts_events", "Friendly script events", handlerBoolPointer, 1, NULL, &CMitigationMgr::bBlockFriendlyScriptedEvents);
		pMenuProtections->AddButton("entry_prot_sync_crash", "Invalid syncs", handlerBoolPointer, 1, NULL, &CMitigationMgr::bSyncCrashProt);
		CFrontend::RegisterMenu(pMenuProtections);

		//network menu
		CMenu* pMenuNetwork = new CMenu("menu_network", "Network", WND_DEFAULT_HEADER_COLOR);
		pMenuNetwork->AddTrigger("entry_network_players", "Players", NULL, 1)->AttachMenu(pMenuPlayerlist);
		pMenuNetwork->AddTrigger("entry_network_prots", "Protections", NULL, 1)->AttachMenu(pMenuProtections);
		//pMenuNetwork->AddTrigger("entry_network_migrate_sc_host", "Migrate script host", handlerMigrateScriptHost, 60);
		pMenuNetwork->AddButton("entry_network_migrate_crash_lobby", "Spawn crash objects", handlerBoolPointer, 1, NULL, &CMitigationMgr::bSpawnCrashObjects);
		CFrontend::RegisterMenu(pMenuNetwork);

#ifdef EX_MODE
		CMenu* pMenuDebug = new CMenu("menu_debug", "Debug", WND_DEFAULT_HEADER_COLOR);
		pMenuDebug->AddButton("entry_debug_log_sc_events",
			"Log scripted events", handlerBoolPointer, 1, NULL, &CMitigationMgr::bLogScriptEvents);
		pMenuDebug->AddButton("entry_debug_log_sync_tree",
			"Log sync trees", handlerBoolPointer, 1, NULL, &CMitigationMgr::bLogSyncTree);
		pMenuDebug->AddTrigger("entry_debug_clear_console", "Clear console", handlerClearConsole, 1);
		CFrontend::RegisterMenu(pMenuDebug);
#endif

		//-----------------------networked players menus-------------------

		CNetworkPlayerMenu* pMenuAbusive = new CNetworkPlayerMenu("menu_networked_abusive", "Abusive", WND_DEFAULT_HEADER_COLOR);
		CPlayerSliderEntry* pExplodeSlider = pMenuAbusive->AddNetworkSlider(
			"entry_networked_player_abusive_explode", "Explode", handlerExplodePlayer, 12);

		pExplodeSlider->AddValue("Grenade", eExplosionTag::EXP_TAG_GRENADE, TRUE);
		pExplodeSlider->AddValue("APC missile", eExplosionTag::EXP_TAG_APCSHELL, TRUE);
		pExplodeSlider->AddValue("Barrel", eExplosionTag::EXP_TAG_BARREL, TRUE);
		pExplodeSlider->AddValue("Bike", eExplosionTag::EXP_TAG_BIKE, TRUE);
		pExplodeSlider->AddValue("Bird Crap", eExplosionTag::EXP_TAG_BIRD_CRAP, TRUE);
		pExplodeSlider->AddValue("Blimp", eExplosionTag::EXP_TAG_BLIMP, TRUE);
		pExplodeSlider->AddValue("Boat", eExplosionTag::EXP_TAG_BOAT, TRUE);
		pExplodeSlider->AddValue("Bombushka", eExplosionTag::EXP_TAG_BOMBUSHKA_CANNON, TRUE);
		pExplodeSlider->AddValue("Bomb (cluster)", eExplosionTag::EXP_TAG_BOMB_CLUSTER, TRUE);
		pExplodeSlider->AddValue("Bomb (cluster 2)", eExplosionTag::EXP_TAG_BOMB_CLUSTER_SECONDARY, TRUE);
		pExplodeSlider->AddValue("Bomb (gas)", eExplosionTag::EXP_TAG_BOMB_GAS, TRUE);
		pExplodeSlider->AddValue("Bomb (incendiary)", eExplosionTag::EXP_TAG_BOMB_INCENDIARY, TRUE);
		pExplodeSlider->AddValue("Bomb (standard)", eExplosionTag::EXP_TAG_BOMB_STANDARD, TRUE);
		pExplodeSlider->AddValue("Bomb (standard2)", eExplosionTag::EXP_TAG_BOMB_STANDARD_WIDE, TRUE);
		pExplodeSlider->AddValue("Bomb (water)", eExplosionTag::EXP_TAG_BOMB_WATER, TRUE);
		pExplodeSlider->AddValue("Bullet", eExplosionTag::EXP_TAG_BULLET, TRUE);
		pExplodeSlider->AddValue("Mine", eExplosionTag::EXP_TAG_BURIEDMINE, TRUE);
		pExplodeSlider->AddValue("Underwater mine", eExplosionTag::EXP_TAG_MINE_UNDERWATER, TRUE);
		pExplodeSlider->AddValue("BZ gas", eExplosionTag::EXP_TAG_BZGAS, TRUE);
		pExplodeSlider->AddValue("Car", eExplosionTag::EXP_TAG_CAR, TRUE);
		pExplodeSlider->AddValue("Flame", eExplosionTag::EXP_TAG_DIR_FLAME, TRUE);
		pExplodeSlider->AddValue("Flame 2", eExplosionTag::EXP_TAG_DIR_FLAME_EXPLODE, TRUE);
		pExplodeSlider->AddValue("Gas canister", eExplosionTag::EXP_TAG_DIR_GAS_CANISTER, TRUE);
		pExplodeSlider->AddValue("Steam", eExplosionTag::EXP_TAG_DIR_STEAM, TRUE);
		pExplodeSlider->AddValue("Hydrant", eExplosionTag::EXP_TAG_DIR_WATER_HYDRANT, TRUE);
		pExplodeSlider->AddValue("Explosive ammo", eExplosionTag::EXP_TAG_EXPLOSIVEAMMO, TRUE);
		pExplodeSlider->AddValue("Explosive shotgun", eExplosionTag::EXP_TAG_EXPLOSIVEAMMO_SHOTGUN, TRUE);
		pExplodeSlider->AddValue("Extinguisher", eExplosionTag::EXP_TAG_EXTINGUISHER, TRUE);
		pExplodeSlider->AddValue("Firework", eExplosionTag::EXP_TAG_FIREWORK, TRUE);
		pExplodeSlider->AddValue("Flare", eExplosionTag::EXP_TAG_FLARE, TRUE);
		pExplodeSlider->AddValue("Flash grenade", eExplosionTag::EXP_TAG_FLASHGRENADE, TRUE);
		pExplodeSlider->AddValue("Gas tank", eExplosionTag::EXP_TAG_GAS_TANK, TRUE);
		pExplodeSlider->AddValue("Grenade launcher", eExplosionTag::EXP_TAG_GRENADELAUNCHER, TRUE);
		pExplodeSlider->AddValue("HI Octane", eExplosionTag::EXP_TAG_HI_OCTANE, TRUE);
		pExplodeSlider->AddValue("Hunter cannon", eExplosionTag::EXP_TAG_HUNTER_CANNON, TRUE);
		pExplodeSlider->AddValue("Valkyrie cannon", eExplosionTag::EXP_TAG_VALKYRIE_CANNON, TRUE);
		pExplodeSlider->AddValue("Molotov", eExplosionTag::EXP_TAG_MOLOTOV, TRUE);
		pExplodeSlider->AddValue("Kinetic", eExplosionTag::EXP_TAG_MORTAR_KINETIC, TRUE);
		pExplodeSlider->AddValue("Explosive MG", eExplosionTag::EXP_TAG_OPPRESSOR2_CANNON, TRUE);
		pExplodeSlider->AddValue("Explosive MG 2", eExplosionTag::EXP_TAG_ROGUE_CANNON, TRUE);
		pExplodeSlider->AddValue("Orbital cannon", eExplosionTag::EXP_TAG_ORBITAL_CANNON, TRUE);
		pExplodeSlider->AddValue("Air defence", eExplosionTag::EXP_TAG_AIR_DEFENCE, TRUE);
		pExplodeSlider->AddValue("Petrol pump", eExplosionTag::EXP_TAG_PETROL_PUMP, TRUE);
		pExplodeSlider->AddValue("Pipe bomb", eExplosionTag::EXP_TAG_PIPEBOMB, TRUE);
		pExplodeSlider->AddValue("Plane", eExplosionTag::EXP_TAG_PLANE, TRUE);
		pExplodeSlider->AddValue("Plane missile", eExplosionTag::EXP_TAG_PLANE_ROCKET, TRUE);
		pExplodeSlider->AddValue("Propane", eExplosionTag::EXP_TAG_PROPANE, TRUE);
		pExplodeSlider->AddValue("Proximity mine", eExplosionTag::EXP_TAG_PROXMINE, TRUE);
		pExplodeSlider->AddValue("Railgun", eExplosionTag::EXP_TAG_RAILGUN, TRUE);
		pExplodeSlider->AddValue("Raygun", eExplosionTag::EXP_TAG_RAYGUN, TRUE);
		pExplodeSlider->AddValue("RC tank", eExplosionTag::EXP_TAG_RCTANK_ROCKET, TRUE);
		pExplodeSlider->AddValue("Missile", eExplosionTag::EXP_TAG_ROCKET, TRUE);
		pExplodeSlider->AddValue("Ship", eExplosionTag::EXP_TAG_SHIP_DESTROY, TRUE);
		pExplodeSlider->AddValue("Smoke", eExplosionTag::EXP_TAG_SMOKEGRENADE, TRUE);
		pExplodeSlider->AddValue("Drone", eExplosionTag::EXP_TAG_SCRIPT_DRONE, TRUE);
		pExplodeSlider->AddValue("Drone missile", eExplosionTag::EXP_TAG_SCRIPT_MISSILE, TRUE);
		pExplodeSlider->AddValue("Snowball", eExplosionTag::EXP_TAG_SNOWBALL, TRUE);
		pExplodeSlider->AddValue("Sticky bomb", eExplosionTag::EXP_TAG_STICKYBOMB, TRUE);
		pExplodeSlider->AddValue("Stun", eExplosionTag::EXP_TAG_STUNGRENADE, TRUE);
		pExplodeSlider->AddValue("Submarine rocket", eExplosionTag::EXP_TAG_SUBMARINE_BIG, TRUE);
		pExplodeSlider->AddValue("Tanker", eExplosionTag::EXP_TAG_TANKER, TRUE);
		pExplodeSlider->AddValue("Tankshell", eExplosionTag::EXP_TAG_TANKSHELL, TRUE);
		pExplodeSlider->AddValue("Torpedo", eExplosionTag::EXP_TAG_TORPEDO, TRUE);
		pExplodeSlider->AddValue("Train", eExplosionTag::EXP_TAG_TRAIN, TRUE);
		pExplodeSlider->AddValue("Truck", eExplosionTag::EXP_TAG_TRUCK, TRUE);
		pExplodeSlider->AddValue("EMP", eExplosionTag::EXP_TAG_VEHICLEMINE_EMP, TRUE);
		pExplodeSlider->AddValue("Spike", eExplosionTag::EXP_TAG_VEHICLEMINE_SPIKE, TRUE);
		pExplodeSlider->AddValue("Tar", eExplosionTag::EXP_TAG_VEHICLEMINE_TAR, TRUE);
		pMenuAbusive->AddButton("entry_networked_player_abusive_stealth", "Stealth Explosions", handlerBoolPointer, 1, NULL, &bStealthExplosions);
		pMenuAbusive->AddNetworkTrigger("entry_networked_player_abusive_island", "Send to island", handlerSendToIsland, 1);
		pMenuAbusive->AddNetworkTrigger("entry_networked_player_abusive_island", "Kick from CEO", handlerCeoKick, 1);
		pMenuAbusive->AddNetworkTrigger("entry_networked_player_abusive_island", "Ban from CEO", handlerCeoBan, 1);
		CFrontend::RegisterMenu(pMenuAbusive);

		//playerinfo menu
		CNetworkPlayerMenu* pMenuPlayerInfo = new CNetworkPlayerMenu("menu_playerinfo", "Player data", WND_DEFAULT_HEADER_COLOR, OnOpenMenuPlayerInfo);
		pMenuPlayerInfo->AddText("menu_playerinfo_name", "Name", "");
		pMenuPlayerInfo->AddText("menu_playerinfo_ipv4", "IPv4", "");
		pMenuPlayerInfo->AddText("menu_playerinfo_rid", "Rockstar ID", "");
		pMenuPlayerInfo->AddText("menu_playerinfo_ht", "Host token", "");
		pMenuPlayerInfo->AddText("menu_playerinfo_id", "Lobby id", "");
#ifdef EX_MODE
		pMenuPlayerInfo->AddNetworkTrigger("entry_playerinfo_get_class", "Print ped class", handlerPrintPedClass, 1);
		pMenuPlayerInfo->AddNetworkTrigger("entry_playerinfo_get_cnetgameplayer", "Print CNetGamePlayer", handlerPrintCNetGamePlayer, 1);
		pMenuPlayerInfo->AddNetworkTrigger("entry_playerinfo_get_netplayerdata", "Print netPlayerData", handlerPrintNetPlayerData, 1);
#endif
		CFrontend::RegisterMenu(pMenuPlayerInfo);

		//removals menu
		CNetworkPlayerMenu* pMenuRemovals = new CNetworkPlayerMenu("menu_networked_removals", "Removals", WND_DEFAULT_HEADER_COLOR);
		pMenuRemovals->AddNetworkTrigger("entry_networked_removals_host_kick", "Host kick", handleHostKick, 60);
		pMenuRemovals->AddNetworkTrigger("entry_networked_removals_break_game", "Break freemode kick", handlerBreakFreemodeKick, 360);
		pMenuRemovals->AddNetworkTrigger("entry_networked_removals_desync_kick", "Desync kick", handlerDesyncKick, 1);
		pMenuRemovals->AddNetworkTrigger("entry_networked_removals_crashes", "Crash", handlerCrashPlayer, 240);
		CFrontend::RegisterMenu(pMenuRemovals);

		//trolling menu
		CNetworkPlayerMenu* pMenuTrolling = new CNetworkPlayerMenu("menu_networked_trolling", "Trolling", WND_DEFAULT_HEADER_COLOR);
		pMenuTrolling->AddNetworkTrigger("entry_networked_trllng_kick_veh", "Kick from vehicle", handlerKickFromVehicle, 30);
		CPlayerSliderEntry* pSliderRmWeaponGroup = pMenuTrolling->AddNetworkSlider("entry_networked_trllng_rm_wpgroup",
			"Remove weapon group", handlerRemoveWeaponGroup, 1);
		pSliderRmWeaponGroup->AddValue("Pistols", wpPISTOL, TRUE);
		pSliderRmWeaponGroup->AddValue("MGs", wpMG, TRUE);
		pSliderRmWeaponGroup->AddValue("Rifles", wpRIFLE, TRUE);
		pSliderRmWeaponGroup->AddValue("Snipers", wpSNIPER, TRUE);
		pSliderRmWeaponGroup->AddValue("Melees", wpMELEE, TRUE);
		pSliderRmWeaponGroup->AddValue("Shotguns", wpSHOTGUN, TRUE);
		pSliderRmWeaponGroup->AddValue("Heavys", wpHEAVY, TRUE);
		pSliderRmWeaponGroup->AddValue("Specials", wpSPECIAL, TRUE);
		pMenuTrolling->AddNetworkTrigger("entry_networked_trolling_rm_weapons", "Remove weapons", handlerRemoveAllWeapons, 30);
		CFrontend::RegisterMenu(pMenuTrolling);

		//networked player menu
		CNetworkPlayerMenu* pMenuNetPlayer = new CNetworkPlayerMenu("menu_networked_player", "", WND_DEFAULT_HEADER_COLOR);
		pMenuNetPlayer->AddTrigger("entry_networked_player_info", "Info", NULL, 1)->AttachMenu(pMenuPlayerInfo);
		pMenuNetPlayer->AddTrigger("entry_networked_player_abusive", "Abusive", NULL, 1)->AttachMenu(pMenuAbusive);
		pMenuNetPlayer->AddTrigger("entry_networked_player_trolling", "Trolling", NULL, 1)->AttachMenu(pMenuTrolling);
		pMenuNetPlayer->AddTrigger("entry_networked_player_removals", "Removals", NULL, 1)->AttachMenu(pMenuRemovals);
		//pMenuNetPlayer->AddNetworkTrigger("entry_networked_player_tp_vehicle", "Teleport inside vehicle", handlerTeleportInsideVehicle, 1);
		pMenuNetPlayer->AddNetworkButton("entry_networked_player_block_syncs", "Block syncs", handlerBlockSyncs, 1, NULL);
		pMenuNetPlayer->AddNetworkButton("entry_networked_player_spectate", "Spectate player", handlerSpectatePlayer, 1, NULL);
		CFrontend::RegisterMenu(pMenuNetPlayer);

		//Weapon menu
		CMenu* pMenuWeapon = new CMenu("menu_weapon", "Weapon", WND_DEFAULT_HEADER_COLOR);
		pMenuWeapon->AddTrigger("entry_menu_weapon_max_ammo", "Max Ammo", handlerMaxAmmo, 1);
		CFrontend::RegisterMenu(pMenuWeapon);

		//Vehicle menu
		CMenu* pMenuVehicle = new CMenu("menu_vehicle", "Weapon", WND_DEFAULT_HEADER_COLOR);
		pMenuVehicle->AddTrigger("entry_vehicle_fix", "Repair vehicle", handlerRepairVehicle, 1);
		CFrontend::RegisterMenu(pMenuVehicle);

		//main menu
		CMenu* pMenuMain = new CMenu("main", "NoobMenu", WND_DEFAULT_HEADER_COLOR);
		//pMenuMain->AddTrigger("entry_main_player", "Player", NULL, 1)->AttachMenu(pMenuSelf);
		pMenuMain->AddTrigger("entry_main_network", "Network", NULL, 1)->AttachMenu(pMenuNetwork);
		pMenuMain->AddTrigger("entry_main_vehicle", "Vehicle", NULL, 1)->AttachMenu(pMenuVehicle);
		pMenuMain->AddTrigger("entry_main_weapon", "Weapon", NULL, 1)->AttachMenu(pMenuWeapon);
#ifdef EX_MODE
		pMenuMain->AddTrigger("entry_main_debug", "Debug", NULL, 1)->AttachMenu(pMenuDebug);
#endif
		CFrontend::RegisterMenu(pMenuMain);
		CFrontend::SetMainMenu(pMenuMain);
	}
};
