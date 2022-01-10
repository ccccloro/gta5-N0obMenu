#pragma once

#include <Windows.h>
#include "types.h"
#include "classes.h"
#include "config.h"

class CGraphicsMgr
{
public:

	static inline CTextInfo* pTextInfo = NULL;
	static inline RECT srScreenSize;
	static inline FLOAT fMenuX = WND_DEFAULT_OFFSET_X;
	static inline FLOAT fMenuY = WND_DEFAULT_OFFSET_Y;

	static VOID DrawFillRect(FLOAT x, FLOAT y, FLOAT width, FLOAT height, Color color)
	{
		FLOAT relW = width / (FLOAT)srScreenSize.right;
		FLOAT relH = height / (FLOAT)srScreenSize.bottom;
		FLOAT relX = (x / (FLOAT)srScreenSize.right) + relW / (FLOAT)2;
		FLOAT relY = (y / (FLOAT)srScreenSize.bottom) + relH / (FLOAT)2;

		x64::spoof_call(CNativesMgr::draw_rect, &x64::fastcall_ctx, relX, relY, relW, relH, color.r, color.g, color.b, color.a);
	}

	static VOID DrawText(LPCSTR text, FLOAT x, FLOAT y, INT font, FLOAT scale, CColor color, WORD justification)
	{
		FLOAT textX = x / (FLOAT)srScreenSize.right;
		FLOAT textY = y / (FLOAT)srScreenSize.bottom;

		pTextInfo->setColor(color);
		pTextInfo->setScale(scale + .055f * ((1080.f - (FLOAT)srScreenSize.bottom) / 180.f));
		pTextInfo->justification = justification;
		pTextInfo->font = font;
		pTextInfo->wrap_start = 0.f;
		pTextInfo->wrap_end = 1.f;


		x64::spoof_call(CNativesMgr::begin_text_cmd_display_text, &x64::fastcall_ctx, "STRING");
		x64::spoof_call(CNativesMgr::add_text_component_substring_playername, &x64::fastcall_ctx, text);
		x64::spoof_call(CNativesMgr::end_text_cmd_display_text, &x64::fastcall_ctx, textX, textY, 0);
	}

private:

	CGraphicsMgr() {};
	~CGraphicsMgr() {};
};