#include <stdafx.h>

#include "HUD.h"

struct Vector2
{
	float x;
	float y;
};

struct HUDColor
{
	__int8 b;
	__int8 g;
	__int8 r;
	__int8 a;
};

struct HUDComponent
{
	__int8 bActive : 1;
	__int8 bCodeThinksVisible : 1;
	__int8 bHasGfx : 1;
	__int8 bStandaloneMovie : 1;
	__int8 bInUseByActionScript : 1;
	__int8 bWantToResetPositionOnNextSetActive : 1;
	__int8 bIsPositionOverridden : 1;
	__int8 bShouldShowComponentOnMpTextChatClose : 1;
	__int8 bShouldShowComponentOnIMEClose : 1;
	__int32 iLoadingState : 4;
	__int32 iScriptRequestVisibility : 4;
	__int32 iScriptRequestVisibilityPreviousFrame : 4;
	int iId;
	int iDepth;
	int iListId;
	int iListPriority;
	char cFilename[40];
	char cAlignX[2];
	char cAlignY[2];
	Vector2 vPos;
	Vector2 vSize;
	Vector2 vRevisedPos;
	Vector2 vRevisedSize;
	Vector2 vOverriddenPositionToUseNextSetActive;
	float fNoOffsetPosX;
	float fNoOffsetRevisedPosX;
	HUDColor colour;
};

static void (*OG_UpdateHUDComponent)(int id);
static void HK_UpdateHUDComponent(int id)
{
	OG_UpdateHUDComponent(id);
}

static HUDComponent *GetHUDComponent(int id)
{
	static HUDComponent *componentsArray;

	if (!componentsArray)
	{
		Handle handle =
		    Memory::FindPattern("48 8D 35 ?? ?? ?? ?? 84 C0 0F 84 E7 00 00 00", "48 8D 35 ?? ?? ?? ?? 48 8D 0C 33");
		if (handle.IsValid())
		{
			handle          = handle.At(2).Into();
			componentsArray = handle.Get<HUDComponent>();
		}
		else
			return nullptr;

		handle = Memory::FindPattern("E8 ?? ?? ?? ?? 8B 0B 8B C1", "E8 ?? ?? ?? ?? 8B 44 1F FC");
		if (handle.IsValid())
			Memory::AddHook(handle.Into().Get<void>(), HK_UpdateHUDComponent, &OG_UpdateHUDComponent);
	}

	if (id < 0 || id > 160)
		return nullptr;

	return &componentsArray[id];
}