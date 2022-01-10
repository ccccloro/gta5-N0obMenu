#pragma once

#include <vector>
#include <Windows.h>
#include "types.h"
#include "menu.h"
#include "x64tools.h"
#include <string>

struct SPlayerFlags
{
	//network flags
	BOOL isExeCorrupted = FALSE;
	BOOL isModding = FALSE;
	BOOL hasReported = FALSE;
	BOOL scHost = FALSE;
	BOOL isHost = FALSE;
	BOOL bBlockSyncs = FALSE;

	//script flags
	BOOL isSpectated = FALSE;
};

typedef struct _SPlayer
{
	char sPlayerName[32];
	INT iPlayerIndex;
	SPlayerFlags srFlags;
	struct _SPlayer* pNextPlayer = NULL;
} SPlayer;

class CPlayerListMgr
{
public:

	static inline SPlayer* pFirstPlayer = NULL;

public:

	static SPlayer* getPlayer(Player id)
	{
		SPlayer* pCurrentPlayer = pFirstPlayer;
		while (pCurrentPlayer)
		{
			if (pCurrentPlayer->iPlayerIndex == id)
				return pCurrentPlayer;
			pCurrentPlayer = pCurrentPlayer->pNextPlayer;
		}
		return NULL;
	}

	static VOID AddPlayerToList(LPCSTR sPlayerName, INT index)
	{
		SPlayer* pLastPlayer = NULL;
		SPlayer* pCurrentPlayer = pFirstPlayer;

		//get last player from list
		while (pCurrentPlayer)
		{
			pLastPlayer = pCurrentPlayer;
			pCurrentPlayer = pCurrentPlayer->pNextPlayer;
		}

		if (pLastPlayer)
		{
			if ((pLastPlayer->pNextPlayer = (SPlayer*)malloc(sizeof(SPlayer))) != NULL)
			{
				memset(pLastPlayer->pNextPlayer, 0, sizeof(SPlayer));
				memset(&pLastPlayer->pNextPlayer->srFlags, 0, sizeof(pLastPlayer->srFlags));
				memcpy((VOID*)pLastPlayer->pNextPlayer->sPlayerName, (LPCVOID*)sPlayerName, (strlen(sPlayerName) + 1));
				pLastPlayer->pNextPlayer->iPlayerIndex = index;
				pLastPlayer->pNextPlayer->pNextPlayer = NULL;
			}
		}
		else
		{
			if ((pFirstPlayer = (SPlayer*)malloc(sizeof(SPlayer))) != NULL)
			{
				memset(pFirstPlayer, 0, sizeof(SPlayer));
				memset(&pFirstPlayer->srFlags, 0, sizeof(pLastPlayer->srFlags));
				memcpy((VOID*)pFirstPlayer->sPlayerName, (LPCVOID*)sPlayerName, (strlen(sPlayerName) + 1));
				pFirstPlayer->iPlayerIndex = index;
				pFirstPlayer->pNextPlayer = NULL;
			}
		}
	}

	static BOOL IsPlayerInList(LPCSTR sPlayerName)
	{
		SPlayer* pCurrentPlayer = pFirstPlayer;
		while (pCurrentPlayer)
		{
			if (strcmp(pCurrentPlayer->sPlayerName, sPlayerName) == 0)
				return TRUE;
			pCurrentPlayer = pCurrentPlayer->pNextPlayer;
		}
		return FALSE;
	}

	static VOID RemovePlayerFromList(Player id)
	{
		SPlayer* pLastPlayer = NULL;
		SPlayer* pCurrentPlayer = pFirstPlayer;

		while (pCurrentPlayer)
		{
			if (pCurrentPlayer->iPlayerIndex == id)
			{
				if (pLastPlayer == NULL) //first player
				{
					pFirstPlayer = pCurrentPlayer->pNextPlayer;
					free(pCurrentPlayer);
					return;
				}
				else
				{
					pLastPlayer->pNextPlayer = pCurrentPlayer->pNextPlayer;
					free(pCurrentPlayer);
					return;
				}
			}
			pLastPlayer = pCurrentPlayer;
			pCurrentPlayer = pCurrentPlayer->pNextPlayer;
		}
	}

	static INT getPlayerIdByName(LPCSTR sPlayerName)
	{
		SPlayer* pCurrentPlayer = pFirstPlayer;
		while (pCurrentPlayer)
		{
			if (strcmp(pCurrentPlayer->sPlayerName, sPlayerName) == 0)
				return pCurrentPlayer->iPlayerIndex;
			pCurrentPlayer = pCurrentPlayer->pNextPlayer;
		}
		return -1;
	}

	static VOID ClearPlayerList()
	{
		if (pFirstPlayer != NULL)
		{
			SPlayer* pCurrentPlayer = pFirstPlayer;
			SPlayer* next_player = NULL;
			while (pCurrentPlayer)
			{
				next_player = pCurrentPlayer->pNextPlayer;
				free(pCurrentPlayer);
				pCurrentPlayer = next_player;
			}
		}
	}

	static SPlayer* getHost()
	{
		SPlayer* pCurrentPlayer = pFirstPlayer;
		while (pCurrentPlayer)
		{
			if (pCurrentPlayer->srFlags.isHost)
				return pCurrentPlayer;
			pCurrentPlayer = pCurrentPlayer->pNextPlayer;
		}
		return NULL;
	}

	static VOID UpdateStringAttributes(BOOL bAttr, std::string* str, LPCSTR sAttr)
	{
		size_t index = str->find(sAttr);
		if (index != std::string::npos)
		{
			if (!bAttr)
				str->replace(index, strlen(sAttr), "");
			return;
		}
		if (bAttr)
			str->append(sAttr);
	}

	static VOID RefreshPlayerAttributes(Player player)
	{
		SPlayer* pPlayer = getPlayer(player);
		if (pPlayer)
		{
			CMenu* pPlayerMenu = CFrontend::getMenuByName(MENU_PLAYERLIST);
			if (pPlayerMenu)
			{
				LPCSTR name = x64::spoof_call(CNativesMgr::get_player_name, &x64::fastcall_ctx, player);
				CMenuEntry* pPlayerEntry = pPlayerMenu->getEntryByName(name);
				if (pPlayerEntry)
				{
					SPlayerFlags flags = pPlayer->srFlags;
					std::string newDisplayName = name;
					UpdateStringAttributes(flags.isModding, &newDisplayName, " ~r~[M]");
					UpdateStringAttributes(flags.hasReported, &newDisplayName, " ~p~[R]");
					UpdateStringAttributes(flags.isSpectated, &newDisplayName, " ~g~[SPEC]");
					UpdateStringAttributes(flags.scHost, &newDisplayName, " ~o~[SC-H]");
					UpdateStringAttributes(flags.isHost, &newDisplayName, " ~b~[H]");

					pPlayerEntry->setDisplayName(newDisplayName.c_str());
				}
			}
		}
	}

private:

	CPlayerListMgr() {};
	~CPlayerListMgr() {};
};