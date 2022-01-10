#pragma once

#include <Windows.h>
#include <vector>
#include <stack>
#include "game.h"
#include "types.h"
#include "config.h"

enum eMenuDir
{
	UP,
	DOWN,
	RIGHT,
	LEFT
};

enum eEntryType
{
	BUTTON,
	TOGGLE,
	PLAYER_TRIGGER,
	PLAYER_TOGGLE,
	SLIDER,
	PLAYER_SLIDER_TRIGGER,
	PLAYER_SLIDER_TOGGLE,
	TEXT
};

enum eMenuType
{
	NORMAL,
	PLAYER
};

struct SSliderValue
{
	CHAR sValueName[32];
	INT iValue;
	BOOL bShouldDisplayName;
};

class CMenu;
class CMenuEntry;
class CSliderEntry;
class CPlayerSliderEntry;
class CNetworkPlayerMenu;

using EntryHandler = VOID(__fastcall*)(CMenuEntry*);
using PlayerEntryHandler = VOID(__fastcall*)(CMenuEntry* entry, Player id);
using EntrySliderHandler = VOID(__fastcall*)(CSliderEntry* entry);
using EntryPlayerSliderHandler = VOID(__fastcall*)(CPlayerSliderEntry* entry, Player id);
using NetworkMenuOnOpenHandler = VOID(__fastcall*)(CNetworkPlayerMenu* thisptr, Player id);

class CMenuEntry
{
public:
	explicit CMenuEntry(LPCSTR entry_name, LPCSTR display_name, eEntryType type) : _type(type), pAttachedMenu(NULL), bActive(FALSE)
	{
		memcpy(name, entry_name, strlen(entry_name) + 1);
		memcpy(sDisplayName, display_name, strlen(display_name) + 1);
	}
	explicit CMenuEntry(LPCSTR entry_name, LPCSTR display_name, eEntryType type, BOOL* bStatus) : _type(type), pAttachedMenu(NULL), bActive(FALSE)
	{
		memcpy(name, entry_name, strlen(entry_name) + 1);
		memcpy(sDisplayName, display_name, strlen(display_name) + 1);
		_bStatus = bStatus;
	}

	virtual ~CMenuEntry() {};

	virtual LPCSTR getName() { return name; }
	virtual LPCSTR getDisplayName() { return sDisplayName; }

	void setDisplayName(LPCSTR newName)
	{
		memset(sDisplayName, 0, sizeof(sDisplayName));
		memcpy(sDisplayName, newName, strlen(newName) + 1);
	}

	VOID setActive(BOOL toggle) { bActive = toggle; }
	BOOL isActive() 
	{ 
		if (_bStatus != NULL)
			return *_bStatus;
		return bActive; 
	}

	VOID setStatusPtr(BOOL* bStatus) { _bStatus = bStatus; }
	BOOL* getStatusPtr() { return _bStatus; }

	eEntryType get_type() { return _type; }

protected:

	virtual VOID _trigger() {}
	virtual VOID _trigger(Player id) {} //for network entries

public:

	virtual VOID trigger() { _trigger(); }
	virtual VOID trigger(Player id) { _trigger(id); }

	VOID AttachMenu(CMenu* menu) { pAttachedMenu = menu; }
	CMenu* get_submenu() { return pAttachedMenu; }

private:
	char name[64];
	char sDisplayName[64];
	eEntryType _type;
	CMenu* pAttachedMenu;
	BOOL bActive;
	BOOL* _bStatus = NULL;
};

class CNetworkMenuEntry : public CMenuEntry
{
public:

	explicit CNetworkMenuEntry(LPCSTR entry_name, LPCSTR display_name, eEntryType type)
		: CMenuEntry(entry_name, display_name, type) {}
	explicit CNetworkMenuEntry(LPCSTR entry_name, LPCSTR display_name, eEntryType type, BOOL* bStatus)
		: CMenuEntry(entry_name, display_name, type, bStatus) {}
	
	virtual ~CNetworkMenuEntry() {};

public:

	VOID trigger(Player id)
	{
		_trigger(id);
	}
};

class CTextEntry : public CMenuEntry
{
public:

	/*format: "header: label"*/
	explicit CTextEntry(LPCSTR entry_name, LPCSTR header, LPCSTR label)
		: CMenuEntry(entry_name, header, eEntryType::TEXT)
	{
		memcpy(sHeaderBuff, header, strlen(header) + 1);
		sprintf_s(sTextBuff, "%s: %s", header, label);
	}
	~CTextEntry() {}

	LPCSTR getDisplayName()
	{
		return sTextBuff;
	}

	VOID setLabel(LPCSTR label)
	{
		sprintf_s(sTextBuff, "%s: %s", sHeaderBuff, label);
	}

private:

	char sHeaderBuff[32];
	char sTextBuff[48];
};

class CTriggerEntry : public CMenuEntry
{
public:

	explicit CTriggerEntry(LPCSTR entry_name, LPCSTR display_name, uint32_t coolDown, EntryHandler handler)
		: CMenuEntry(entry_name, display_name, eEntryType::BUTTON), _handler(handler), _coolDown(coolDown), bEnabled(TRUE), lastFramePressed(0) {}
	~CTriggerEntry() {}

	VOID _trigger()
	{
		if (bEnabled)
		{
			if (*CMemoryMgr::tick_count - lastFramePressed > _coolDown)
			{
				_handler(this); //triggers the button
				lastFramePressed = *CMemoryMgr::tick_count;
			}
		}
	}

	VOID enable(BOOL toggle) { bEnabled = toggle; }

private:

	EntryHandler _handler;
	uint32_t lastFramePressed;
	uint32_t _coolDown;
	BOOL bEnabled;
};

class CToggleEntry : public CMenuEntry
{
public:
	explicit CToggleEntry(LPCSTR entry_name, LPCSTR display_name, uint32_t coolDown, EntryHandler handler, uint32_t id, BOOL* bStatus)
		: CMenuEntry(entry_name, display_name, eEntryType::TOGGLE, bStatus), _handler(handler), lastFramePressed(0), _coolDown(coolDown), _id(id) {}
	explicit CToggleEntry(LPCSTR entry_name, LPCSTR display_name, uint32_t coolDown, EntryHandler handler, uint32_t id)
		: CMenuEntry(entry_name, display_name, eEntryType::TOGGLE), _handler(handler), lastFramePressed(0), _coolDown(coolDown), _id(id) {}
	explicit CToggleEntry(LPCSTR entry_name, LPCSTR display_name, uint32_t coolDown, EntryHandler handler)
		: CMenuEntry(entry_name, display_name, eEntryType::TOGGLE), _handler(handler), lastFramePressed(0), _coolDown(coolDown), _id(0) {}
	~CToggleEntry() {}

	VOID _trigger()
	{
		if (*CMemoryMgr::tick_count - lastFramePressed > _coolDown)
		{
			_handler(this); //triggers the button
			lastFramePressed = *CMemoryMgr::tick_count;
		}
	}

	uint32_t getId() { return _id; }

private:

	EntryHandler _handler;
	uint32_t lastFramePressed;
	uint32_t _coolDown;
	uint32_t _id;
};

class CPlayerTriggerEntry : public CNetworkMenuEntry
{
public:

	explicit CPlayerTriggerEntry(LPCSTR entry_name, LPCSTR display_name, uint32_t coolDown, PlayerEntryHandler handler)
		: CNetworkMenuEntry(entry_name, display_name, eEntryType::PLAYER_TRIGGER), _handler(handler), _coolDown(coolDown), bEnabled(TRUE), lastFramePressed(0) {}
	~CPlayerTriggerEntry() {}

	VOID _trigger(Player id)
	{
		if (bEnabled)
		{
			if (*CMemoryMgr::tick_count - lastFramePressed > _coolDown)
			{
				_handler(this, id); //triggers the button
				lastFramePressed = *CMemoryMgr::tick_count;
			}
		}
	}

	VOID enable(BOOL toggle) { bEnabled = toggle; }

private:

	PlayerEntryHandler _handler;
	uint32_t lastFramePressed;
	uint32_t _coolDown;
	BOOL bEnabled;
};

class CPlayerToggleEntry : public CNetworkMenuEntry
{
public:
	explicit CPlayerToggleEntry(LPCSTR entry_name, LPCSTR display_name, uint32_t coolDown, PlayerEntryHandler handler)
		: CNetworkMenuEntry(entry_name, display_name, eEntryType::PLAYER_TOGGLE), _handler(handler), lastFramePressed(0), _coolDown(coolDown) {}
	explicit CPlayerToggleEntry(LPCSTR entry_name, LPCSTR display_name, uint32_t coolDown, PlayerEntryHandler handler, BOOL* bStatus)
		: CNetworkMenuEntry(entry_name, display_name, eEntryType::PLAYER_TOGGLE, bStatus), _handler(handler), lastFramePressed(0), _coolDown(coolDown) {}
	~CPlayerToggleEntry() {}

	VOID _trigger(Player id)
	{
		if (*CMemoryMgr::tick_count - lastFramePressed > _coolDown)
		{
			_handler(this, id); //triggers the button
			lastFramePressed = *CMemoryMgr::tick_count;
		}
	}

private:

	PlayerEntryHandler _handler;
	uint32_t lastFramePressed;
	uint32_t _coolDown;
};

class CSliderEntry : public CMenuEntry
{
public:

	explicit CSliderEntry(eEntryType sliderType, LPCSTR entry_name, LPCSTR display_name, uint32_t coolDown, EntrySliderHandler handler)
		: CMenuEntry(entry_name, display_name, sliderType), _handler(handler),
		lastFramePressed(0), _coolDown(coolDown), iValueIndex(0) {}

	explicit CSliderEntry(LPCSTR entry_name, LPCSTR display_name, uint32_t coolDown, EntrySliderHandler handler)
		: CMenuEntry(entry_name, display_name, eEntryType::SLIDER), _handler(handler),
		lastFramePressed(0), _coolDown(coolDown), iValueIndex(0) {}
	virtual ~CSliderEntry()
	{
		for (DWORD i = 0; i < aValuesList.size(); i++)
		{
			SSliderValue* curr_value = aValuesList.at(i);
			if (curr_value)
				free(curr_value);
		}
	}

	VOID _trigger()
	{
		if (*CMemoryMgr::tick_count - lastFramePressed > _coolDown)
		{
			if (_handler != NULL) _handler(this);
			lastFramePressed = *CMemoryMgr::tick_count;
		}
	}

	VOID AddValue(LPCSTR name, INT value, BOOL display_name)
	{
		SSliderValue* new_value = (SSliderValue*)malloc(sizeof(SSliderValue));
		if (new_value)
		{
			memcpy(new_value->sValueName, name, strlen(name) + 1);
			new_value->iValue = value;
			new_value->bShouldDisplayName = display_name;
			aValuesList.push_back(new_value);
		}
	}

	virtual VOID setIndex(INT new_index)
	{
		INT max_values = aValuesList.size();
		if (new_index >= max_values)
			iValueIndex = 0;
		else if (new_index < 0)
			iValueIndex = max_values - 1;
		else
			iValueIndex = new_index;
	}

	INT getIndex() { return iValueIndex; }

	SSliderValue* get_current_value()
	{
		if (!aValuesList.empty())
			return aValuesList.at(iValueIndex);
		return NULL;
	}

protected:

	EntrySliderHandler _handler;
	uint32_t lastFramePressed;
	uint32_t _coolDown;
	std::vector<SSliderValue*> aValuesList;
	INT iValueIndex;
};

class CSliderToggleEntry : public CSliderEntry
{
public:

	explicit CSliderToggleEntry(LPCSTR entry_name, LPCSTR display_name, uint32_t coolDown, EntrySliderHandler handler, EntrySliderHandler onChange)
		: CSliderEntry(eEntryType::PLAYER_SLIDER_TOGGLE, entry_name, display_name, coolDown, handler), _on_change_handler(onChange) {}
	~CSliderToggleEntry() {}

	VOID setIndex(INT new_index)
	{
		INT max_values = aValuesList.size();
		if (new_index >= max_values)
			iValueIndex = 0;
		else if (new_index < 0)
			iValueIndex = max_values - 1;
		else
			iValueIndex = new_index;
		if (_on_change_handler != NULL)
			_on_change_handler(this);
	}

private:

	EntrySliderHandler _on_change_handler;
};

class CPlayerSliderEntry : public CNetworkMenuEntry
{
public:
	explicit CPlayerSliderEntry(LPCSTR entry_name, LPCSTR display_name, uint32_t coolDown, EntryPlayerSliderHandler handler)
		: CNetworkMenuEntry(entry_name, display_name, eEntryType::PLAYER_SLIDER_TRIGGER), _handler(handler),
		lastFramePressed(0), _coolDown(coolDown), iValueIndex(0) {}
	~CPlayerSliderEntry()
	{
		for (DWORD i = 0; i < aValuesList.size(); i++)
		{
			SSliderValue* curr_value = aValuesList.at(i);
			if (curr_value)
				free(curr_value);
		}
	}

	VOID _trigger(Player id)
	{
		if (*CMemoryMgr::tick_count - lastFramePressed > _coolDown)
		{
			_handler(this, id);
			lastFramePressed = *CMemoryMgr::tick_count;
		}
	}

	VOID AddValue(LPCSTR name, INT value, BOOL display_name)
	{
		SSliderValue* pNewValue = (SSliderValue*)malloc(sizeof(SSliderValue));
		if (pNewValue)
		{
			memcpy(pNewValue->sValueName, name, strlen(name) + 1);
			pNewValue->iValue = value;
			pNewValue->bShouldDisplayName = display_name;
			aValuesList.push_back(pNewValue);
		}
	}

	VOID setIndex(INT new_index)
	{
		INT iMaxValues = aValuesList.size();
		if (new_index >= iMaxValues)
			iValueIndex = 0;
		else if (new_index < 0)
			iValueIndex = iMaxValues - 1;
		else
			iValueIndex = new_index;
	}

	INT getIndex() { return iValueIndex; }

	SSliderValue* get_current_value()
	{
		if (!aValuesList.empty())
			return aValuesList.at(iValueIndex);
		return NULL;
	}

private:

	EntryPlayerSliderHandler _handler;
	uint32_t lastFramePressed;
	uint32_t _coolDown;
	std::vector<SSliderValue*> aValuesList;
	INT iValueIndex;
};



class CMenuWindow
{
public:

	explicit CMenuWindow(LPCSTR window_name, LPCSTR display_name, Color color)
		: fWidth(WND_WIDTH), fHeight(0), fX(CGraphicsMgr::fMenuX), fY(CGraphicsMgr::fMenuY),
		srBackground({ 0, 0, 0, 180 }), srHeader(color), index(0), srText({ 255, 255, 255, 255 })
	{
		memcpy(sWindowName, window_name, strlen(window_name) + 1);
		memcpy(sWindowDisplayName, display_name, strlen(display_name) + 1);
	};
	virtual ~CMenuWindow()
	{
		for (DWORD i = 0; i < aEntriesList.size(); i++)
		{
			CMenuEntry* entry = aEntriesList.at(i);
			if (entry)
				delete(entry);
		}
	}

	VOID display()
	{
		CGraphicsMgr::DrawFillRect(fX, fY, fWidth, WND_HEADER_HEIGHT, srHeader);
		CGraphicsMgr::DrawText(sWindowDisplayName, fX + (fWidth / 2), fY + (FLOAT)(WND_HEADER_HEIGHT / 2) - (FLOAT)(WND_HEADER_HEIGHT / 4),
			1, WND_HEADER_TEXT_SCALE, { 255, 255, 255, 255 }, 0);

		CMenuEntry* pEntry = NULL;
		int entryType;
		size_t entries = aEntriesList.size();

		size_t first, last, i = 0;

		if (entries > WND_MAX_ENTRIES)
		{
			if (index > (WND_MAX_ENTRIES - 1))
			{
				first = index - (WND_MAX_ENTRIES - 1);
				last = first + WND_MAX_ENTRIES;
			}
			else
			{
				first = 0;
				last = WND_MAX_ENTRIES;
			}
		}
		else
		{
			first = 0;
			last = entries;
		}

		for (; first < last; first++)
		{
			pEntry = aEntriesList.at(first);
			entryType = pEntry->get_type();

			if (first == index)
			{
				srText = { 0, 0, 0, 255 };
				srBackground = { 255, 255, 255, 180 };
			}
			else
			{
				srText = { 255, 255, 255, 255 };
				srBackground = { 0, 0, 0, 180 };
			}

			if (entryType == eEntryType::TOGGLE || entryType == eEntryType::PLAYER_SLIDER_TOGGLE || entryType == eEntryType::PLAYER_TOGGLE)
			{
				if (pEntry->isActive())
					srText = { 0, 255, 0, 255 };
				else
					srText = {223, 99, 99, 255};
			}

			CGraphicsMgr::DrawFillRect(fX, fY + WND_HEADER_HEIGHT + i * WND_ENTRY_HEIGHT, fWidth, WND_ENTRY_HEIGHT, srBackground);
			CGraphicsMgr::DrawText(pEntry->getDisplayName(), fX + TEXT_LEFT_SPACING,
				fY + WND_HEADER_HEIGHT + i * WND_ENTRY_HEIGHT + (WND_ENTRY_HEIGHT / 2) - (WND_ENTRY_HEIGHT / 3.5f), 0, WND_ENTRY_TEXT_SCALE, srText, 1);

			if (entryType == eEntryType::SLIDER || entryType == eEntryType::PLAYER_SLIDER_TRIGGER || entryType == eEntryType::PLAYER_SLIDER_TOGGLE)
			{
				SSliderValue* curr_value = NULL;
				if (entryType == SLIDER)
					curr_value = reinterpret_cast<CSliderEntry*>(pEntry)->get_current_value();
				else if (entryType == PLAYER_SLIDER_TRIGGER)
					curr_value = reinterpret_cast<CPlayerSliderEntry*>(pEntry)->get_current_value();
				else
					curr_value = reinterpret_cast<CSliderToggleEntry*>(pEntry)->get_current_value();

				if (curr_value != NULL)
				{
					CHAR buff[32];
					if (curr_value->bShouldDisplayName)
						sprintf(buff, "<%s>", curr_value->sValueName);
					else
						sprintf(buff, "<%d>", curr_value->iValue);

					CGraphicsMgr::DrawText(buff, fX + 3*(fWidth / 4) + 5.f,
						fY + WND_HEADER_HEIGHT + i * WND_ENTRY_HEIGHT + (WND_ENTRY_HEIGHT / 2) - (WND_ENTRY_HEIGHT / 3.5f), 0, WND_ENTRY_TEXT_SCALE, srText, 0);
				}
			}
			i++;
		}

	}

	LPCSTR getName() { return sWindowName; }
	VOID setDisplayName(LPCSTR newName)
	{
		memset(sWindowDisplayName, 0, sizeof(sWindowDisplayName));
		memcpy(sWindowDisplayName, newName, strlen(newName) + 1);
	}

	inline VOID setCoords(FLOAT xc, FLOAT yc)
	{
		fX = xc;
		fY = yc;
	}

protected:

	std::vector<CMenuEntry*> aEntriesList;

private:

	//window info
	char sWindowName[32];
	char sWindowDisplayName[32];
	FLOAT fWidth, fHeight;
	FLOAT fX, fY;

	//color components
	Color srBackground, srHeader;
	CColor srText;

protected:
	size_t index;
};

class CMenu : public CMenuWindow
{
public:

	explicit CMenu(LPCSTR name, LPCSTR display_name, Color color, eMenuType menutype)
		: CMenuWindow(name, display_name, color), iEntriesNum(0), type(menutype) {}
	explicit CMenu(LPCSTR name, LPCSTR display_name, Color color)
		: CMenuWindow(name, display_name, color), iEntriesNum(0), type(eMenuType::NORMAL) {}
	virtual ~CMenu() {}

	virtual VOID TriggerEntry()
	{
		CMenuEntry* entry = aEntriesList.at(index);
		if (entry)
			entry->trigger();
	}

	CTriggerEntry* AddTrigger(LPCSTR name, LPCSTR displayName, EntryHandler handler, uint32_t coolDown)
	{
		CMenuEntry* entry = new CTriggerEntry(name, displayName, coolDown, handler);
		if (entry)
		{
			aEntriesList.push_back(entry);
			iEntriesNum++;
			return reinterpret_cast<CTriggerEntry*>(entry);
		}
		return NULL;
	}

	CToggleEntry* AddButton(LPCSTR name, LPCSTR displayName, EntryHandler handler, uint32_t coolDown)
	{
		CMenuEntry* entry = new CToggleEntry(name, displayName, coolDown, handler);
		if (entry)
		{
			aEntriesList.push_back(entry);
			iEntriesNum++;
			return reinterpret_cast<CToggleEntry*>(entry);
		}
		return NULL;
	}

	CToggleEntry* AddButton(LPCSTR name, LPCSTR displayName, EntryHandler handler, uint32_t coolDown, uint32_t id)
	{
		CMenuEntry* entry = new CToggleEntry(name, displayName, coolDown, handler, id);
		if (entry)
		{
			aEntriesList.push_back(entry);
			iEntriesNum++;
			return reinterpret_cast<CToggleEntry*>(entry);
		}
		return NULL;
	}

	CToggleEntry* AddButton(LPCSTR name, LPCSTR displayName, EntryHandler handler, uint32_t coolDown, uint32_t id, BOOL* status)
	{
		CMenuEntry* entry = new CToggleEntry(name, displayName, coolDown, handler, id, status);
		if (entry)
		{
			aEntriesList.push_back(entry);
			iEntriesNum++;
			return reinterpret_cast<CToggleEntry*>(entry);
		}
		return NULL;
	}

	CSliderEntry* AddSlider(LPCSTR name, LPCSTR displayName, EntrySliderHandler handler, uint32_t coolDown)
	{
		CMenuEntry* entry = new CSliderEntry(name, displayName, coolDown, handler);
		if (entry)
		{
			aEntriesList.push_back(entry);
			iEntriesNum++;
			return reinterpret_cast<CSliderEntry*>(entry);
		}
		return NULL;
	}

	CSliderEntry* AddSlider(LPCSTR name, LPCSTR displayName, EntrySliderHandler handler, EntrySliderHandler onChange, uint32_t coolDown)
	{
		CMenuEntry* entry = new CSliderToggleEntry(name, displayName, coolDown, handler, onChange);
		if (entry)
		{
			aEntriesList.push_back(entry);
			iEntriesNum++;
			return reinterpret_cast<CSliderToggleEntry*>(entry);
		}
		return NULL;
	}

	/*format: "header: label"*/
	CTextEntry* AddText(LPCSTR name, LPCSTR header, LPCSTR label)
	{
		CMenuEntry* entry = new CTextEntry(name, header, label);
		if (entry)
		{
			aEntriesList.push_back(entry);
			iEntriesNum++;
			return reinterpret_cast<CTextEntry*>(entry);
		}
		return NULL;
	}

	VOID RemoveEntry(CMenuEntry* entry)
	{
		if(entry)
		{
			for (DWORD i = 0; i < aEntriesList.size(); i++)
			{
				if (aEntriesList.at(i) == entry)
				{
					delete(entry);
					aEntriesList.erase(aEntriesList.begin() + i);
					if (iEntriesNum > 0) iEntriesNum--;
				}
			}
		}
	}

	size_t getIndex() { return index; }
	INT getEntriesNum() { return iEntriesNum; }
	VOID ResetIndex() { index = 0; }
	VOID SetIndex(INT new_index)
	{
		if (new_index >= iEntriesNum)
			index = 0;
		else if (new_index < 0)
			index = iEntriesNum - 1;
		else
			index = new_index;
	}

	eMenuType getType() { return type; }

	CMenuEntry* getEntryAtIndex(INT index) { return aEntriesList.at(index); }
	CMenuEntry* getEntryByName(LPCSTR name)
	{
		CMenuEntry* entry = NULL;
		for (DWORD i = 0; i < aEntriesList.size(); i++)
		{
			entry = aEntriesList.at(i);
			if (strcmp(entry->getName(), name) == 0)
				return entry;
		}
		return NULL;
	}

protected:

	INT iEntriesNum;
	eMenuType type;
};

class CNetworkPlayerMenu : public CMenu
{
public:

	explicit CNetworkPlayerMenu(LPCSTR name, LPCSTR display_name, Color color, NetworkMenuOnOpenHandler onOpen)
		: CMenu(name, display_name, color, eMenuType::PLAYER), _id(0), onOpenHandler(onOpen) {}

	explicit CNetworkPlayerMenu(LPCSTR name, LPCSTR display_name, Color color)
		: CMenu(name, display_name, color, eMenuType::PLAYER), _id(0), onOpenHandler(NULL) {}
	~CNetworkPlayerMenu() {}

	CPlayerTriggerEntry* AddNetworkTrigger(LPCSTR name, LPCSTR displayName, PlayerEntryHandler handler, uint32_t coolDown)
	{
		CPlayerTriggerEntry* entry = new CPlayerTriggerEntry(name, displayName, coolDown, handler);
		if (entry)
		{
			aEntriesList.push_back(entry);
			iEntriesNum++;
			return reinterpret_cast<CPlayerTriggerEntry*>(entry);
		}
		return NULL;
	}

	CPlayerToggleEntry* AddNetworkButton(LPCSTR name, LPCSTR displayName, PlayerEntryHandler handler, uint32_t coolDown)
	{
		CNetworkMenuEntry* entry = new CPlayerToggleEntry(name, displayName, coolDown, handler);
		if (entry)
		{
			aEntriesList.push_back(entry);
			iEntriesNum++;
			return reinterpret_cast<CPlayerToggleEntry*>(entry);
		}
		return NULL;
	}

	CPlayerToggleEntry* AddNetworkButton(LPCSTR name, LPCSTR displayName, PlayerEntryHandler handler, uint32_t coolDown, BOOL* bStatus)
	{
		CNetworkMenuEntry* entry = new CPlayerToggleEntry(name, displayName, coolDown, handler, bStatus);
		if (entry)
		{
			aEntriesList.push_back(entry);
			iEntriesNum++;
			return reinterpret_cast<CPlayerToggleEntry*>(entry);
		}
		return NULL;
	}

	CPlayerSliderEntry* AddNetworkSlider(LPCSTR name, LPCSTR displayName, EntryPlayerSliderHandler handler, uint32_t coolDown)
	{
		CNetworkMenuEntry* entry = new CPlayerSliderEntry(name, displayName, coolDown, handler);
		if (entry)
		{
			aEntriesList.push_back(entry);
			iEntriesNum++;
			return reinterpret_cast<CPlayerSliderEntry*>(entry);
		}
		return NULL;
	}

	VOID TriggerOnOpenCallback()
	{
		if (onOpenHandler != NULL)
			onOpenHandler(this, _id);
	}

	VOID TriggerEntry()
	{
		CMenuEntry* entry = aEntriesList.at(index);
		if (entry)
		{
			eEntryType type = entry->get_type();
			if (type == PLAYER_TRIGGER || type == PLAYER_TOGGLE || type == PLAYER_SLIDER_TRIGGER)
			{
				reinterpret_cast<CNetworkMenuEntry*>(entry)->trigger(_id);
				return;
			}
			entry->trigger();
		}
	}

	VOID setPlayerId(Player id)
	{
		_id = id;
	}
	Player getPlayerId() { return _id; }

private:

	Player _id;
	NetworkMenuOnOpenHandler onOpenHandler;
};



class CFrontend
{
public:

	static inline BOOL isFrontendActive = FALSE;
	static inline BOOL isMenuActive = FALSE;

	static VOID RegisterMenu(CMenu* menu)
	{
		if (menu)
			aMenusList.push_back(menu);
	}

	static VOID SetMainMenu(CMenu* menu)
	{
		for (DWORD i = 0; i < aMenusList.size(); i++)
		{
			if (aMenusList.at(i) == menu)
			{
				aMenuStack.push(menu);
				break;
			}
		}
	}

	static VOID ShowMenuThisFrame()
	{
		if (!aMenuStack.empty())
			aMenuStack.top()->display();
	}

	static VOID RaiseMenuIndex()
	{
		if (!aMenuStack.empty())
		{
			CMenu* menu = aMenuStack.top();
			INT index = menu->getIndex();
			index++;
			menu->SetIndex(index);
		}
	}

	static VOID LowerMenuIndex()
	{
		if (!aMenuStack.empty())
		{
			CMenu* menu = aMenuStack.top();
			INT index = menu->getIndex();
			index--;
			menu->SetIndex(index);
		}
	}

	static VOID PopMenu()
	{
		if (!aMenuStack.empty())
		{
			if (aMenuStack.size() == 1)
			{
				isMenuActive = FALSE;
				return;
			}
			aMenuStack.pop();
		}
	}

	static VOID PushMenu(CMenu* menu)
	{
		if (!aMenuStack.empty())
		{
			CMenu* currMenu = NULL;
			if (aMenuStack.top() == menu)
				return;
			aMenuStack.push(menu);
		}
	}

	static CMenu* getMenuByName(LPCSTR name)
	{
		CMenu* menu = NULL;
		for (DWORD i = 0; i < aMenusList.size(); i++)
		{
			menu = aMenusList.at(i);
			if (strcmp(menu->getName(), name) == 0)
				return menu;
		}
		return NULL;
	}

	static inline CMenu* getTopMenu()
	{
		if (!aMenuStack.empty())
			return aMenuStack.top();
		return NULL;
	}

	static VOID ReleaseAllFrontend()
	{
		CMenu* menu = NULL;
		for (DWORD i = 0; i < aMenusList.size(); i++)
		{
			menu = aMenusList.at(i);
			if (menu)
				delete(menu);
		}
	}

private:

	static inline std::vector<CMenu*> aMenusList;
	static inline std::stack<CMenu*> aMenuStack;
};