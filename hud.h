#pragma once

#include <Windows.h>
#include <vector>
#include "types.h"
#include "game.h"
#include "graphics.h"
#include "menu.h"

//colors for notifications
enum ENColor
{
	NRED = 1,
	NYELLOW = 2,
	NGREEN = 3,
	NPURPLE = 4,
	NCYAN = 5
};


typedef struct _notification
{
	char text[48];
	char subtext[48];
	INT color;
	size_t curr_frame;
	Vector2 coords;
	INT alpha;
	BOOL isTall;

} SNotification;

class CHudMgr
{
public:

	static VOID refresh()
	{
		//check if notifications are outdated
		SNotification curr_notif;
		Vector2 currCoords, lastCoords;

		for (DWORD i = 0; i < notif_queue.size(); i++)
		{
			curr_notif = notif_queue.at(i);

			if (*CMemoryMgr::tick_count - curr_notif.curr_frame > 550)
				notif_queue.at(i).alpha -= NTF_FADE_RATE;

			if (curr_notif.alpha <= 5)
			{
				currCoords = curr_notif.coords;
				notif_queue.erase(notif_queue.begin() + i);

				//shift all blocks below
				for (DWORD n = i; n < notif_queue.size(); n++)
				{
					if (curr_notif.isTall)
						notif_queue.at(n).coords.y -= 1.5f * NTF_HEIGHT + NTF_Y_SPACE;
					else
						notif_queue.at(n).coords.y -= NTF_HEIGHT + NTF_Y_SPACE;
				}
			}
		}

		//display all notifications
		Color rgba{ 0 };
		size_t notif_len = 0;
		for (DWORD c = 0; c < notif_queue.size(); c++)
		{
			curr_notif = notif_queue.at(c);
			notif_len = strlen(curr_notif.text);

			switch (curr_notif.color)
			{
			case 1:
				rgba = { 255, 0, 0, curr_notif.alpha };
				break;
			case 2:
				rgba = { 255, 255, 0, curr_notif.alpha };
				break;
			case 3:
				rgba = { 0, 255, 0, curr_notif.alpha };
				break;
			case 4:
				rgba = { 155, 3, 73, curr_notif.alpha };
				break;
			case 5:
				rgba = { 0, 230, 230, curr_notif.alpha };
				break;
			default:
				rgba = { 255, 255, 255, curr_notif.alpha };
				break;
			}

			if (CFrontend::isFrontendActive)
			{
				CGraphicsMgr::DrawFillRect(curr_notif.coords.x, curr_notif.coords.y, NTF_WIDTH, NTF_HEIGHT, { 0, 0, 0, rgba.a });
				CGraphicsMgr::DrawFillRect(curr_notif.coords.x, curr_notif.coords.y, NTF_WIDTH, NTF_TOP_HEIGHT, rgba);
				CGraphicsMgr::DrawText(curr_notif.text, curr_notif.coords.x + TEXT_LEFT_SPACING,
					curr_notif.coords.y + (NTF_HEIGHT / 2) - 7.5f, 0, NTF_TEXT_SCALE, { 255, 255, 255, (BYTE)rgba.a }, 1);

				if (curr_notif.isTall)
				{
					//draw the subtext
					CGraphicsMgr::DrawFillRect(curr_notif.coords.x, curr_notif.coords.y + NTF_HEIGHT, NTF_WIDTH, NTF_HEIGHT / 2, { 0, 0, 0, rgba.a });
					CGraphicsMgr::DrawText(curr_notif.subtext, curr_notif.coords.x + TEXT_LEFT_SPACING,
						curr_notif.coords.y + (NTF_HEIGHT / 5) - 12.5f + NTF_HEIGHT, 0, NTF_TEXT_SCALE, { 255, 255, 255, (BYTE)rgba.a }, 1);
				}
			}
		}

		//show menu
		if (CFrontend::isFrontendActive)
			if (CFrontend::isMenuActive)
			{
				CFrontend::getTopMenu()->setCoords(CGraphicsMgr::fMenuX, CGraphicsMgr::fMenuY);
				CFrontend::ShowMenuThisFrame();
			}
	}

	static VOID notify(INT color, LPCSTR text)
	{
		if (CHudMgr::notif_queue.size() >= 10)
			return;

		SNotification notif;
		notif.color = color;
		notif.curr_frame = *CMemoryMgr::tick_count;
		notif.alpha = NTF_ALPHA;
		notif.isTall = FALSE;
		memcpy((VOID*)notif.text, (LPCVOID*)text, (strlen(text) + 1));

		if (does_notification_exists(notif))
			return;

		if (notif_queue.size() == 0)
		{
			notif.coords.x = 24;
			notif.coords.y = 24;
		}
		else
		{
			SNotification last = notif_queue.back();
			if (last.isTall)
			{
				notif.coords.x = last.coords.x;
				notif.coords.y = last.coords.y + 1.5f * NTF_HEIGHT + NTF_Y_SPACE;
			}
			else
			{
				notif.coords.x = last.coords.x;
				notif.coords.y = last.coords.y + NTF_HEIGHT + NTF_Y_SPACE;
			}
		}
		notif_queue.push_back(notif);
	}

	static VOID notify(INT color, LPCSTR text, LPCSTR subtext, ...)
	{

		char buff[48];
		char buff2[48];

		if (CHudMgr::notif_queue.size() >= 10)
			return;

		va_list list;
		va_start(list, subtext);
		vsprintf_s(buff, subtext, list);
		va_end(list);
		sprintf_s(buff2, "%s\n", buff);

		SNotification notif;
		notif.color = color;
		notif.curr_frame = *CMemoryMgr::tick_count;
		notif.alpha = NTF_ALPHA;
		notif.isTall = TRUE;
		memcpy((VOID*)notif.text, (LPCVOID*)text, (strlen(text) + 1));
		memcpy((VOID*)notif.subtext, buff2, (strlen(buff2) + 1));

		if (does_notification_exists(notif))
			return;

		if (notif_queue.size() == 0)
		{
			notif.coords.x = 24;
			notif.coords.y = 24;
		}
		else
		{
			SNotification last = notif_queue.back();
			if (last.isTall)
			{
				notif.coords.x = last.coords.x;
				notif.coords.y = last.coords.y + 1.5f * NTF_HEIGHT + NTF_Y_SPACE;
			}
			else
			{
				notif.coords.x = last.coords.x;
				notif.coords.y = last.coords.y + NTF_HEIGHT + NTF_Y_SPACE;
			}
		}
		notif_queue.push_back(notif);
	}

private:

	static inline std::vector<SNotification> notif_queue;

	static BOOL does_notification_exists(SNotification notif)
	{
		SNotification curr_notif;
		for (DWORD i = 0; i < notif_queue.size(); i++)
		{
			curr_notif = notif_queue.at(i);
			if ((strcmp(curr_notif.text, notif.text) == 0) && (strcmp(curr_notif.subtext, notif.subtext) == 0))
				return TRUE;
		}
		return FALSE;
	}

	CHudMgr() {};
	~CHudMgr() {};
};