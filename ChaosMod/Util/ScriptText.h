#pragma once

#include "Memory/Hooks/TextOutlineHook.h"

#include "Memory/WorldToScreen.h"

#include "Color.h"
#include "Util/Natives.h"

#include <cstdint>

struct ScreenTextVector
{
	const float X;
	const float Y;

	ScreenTextVector(float x, float y) : X(x), Y(y)
	{
	}
};

enum class ScreenTextAdjust
{
	Center,
	Left,
	Right
};

inline float GetScreenTextWidth(const std::string &text, float scale)
{
	_BEGIN_TEXT_COMMAND_GET_WIDTH("STRING");
	ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text.c_str());
	SET_TEXT_SCALE(scale, scale);

	return _END_TEXT_COMMAND_GET_WIDTH(true);
}

inline void DrawScreenText(const std::string &text, const ScreenTextVector &textPos, float scale, Color textColor,
                           bool outline = false, ScreenTextAdjust textAdjust = ScreenTextAdjust::Center,
                           const ScreenTextVector &textWrap = { 0.f, 1.f }, bool addBackground = false)
{
	BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
	ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text.c_str());

	SET_TEXT_SCALE(scale, scale);
	SET_TEXT_COLOUR(textColor.R, textColor.G, textColor.B, textColor.A);

	if (outline)
	{
		Hooks::SetTextOutlineParams(3.f, 50000.f);
		SET_TEXT_OUTLINE();
	}

	SET_TEXT_JUSTIFICATION(static_cast<int>(textAdjust));
	SET_TEXT_WRAP(textWrap.X, textWrap.Y);

	END_TEXT_COMMAND_DISPLAY_TEXT(textPos.X, textPos.Y, 0);
	if (addBackground)
	{
		float width           = GetScreenTextWidth(text, scale);
		float height          = scale / 10;
		float additionalWidth = 0.02f;
		DRAW_RECT(textPos.X - (width * 0.5f), textPos.Y + 0.015, width + additionalWidth, height, 0, 0, 0, 127, true);
	}
}

inline void DrawScreenText(const std::string &text, Color textColor, float x, float y, float z, float size, float maxWidth)
{
	ChaosVector2 screenPos;
	Memory::WorldToScreen({ x, y, z }, &screenPos);
	const auto camCoord = GET_FINAL_RENDERED_CAM_COORD();
	const auto fov      = GET_FINAL_RENDERED_CAM_FOV();
	const auto distance = GET_DISTANCE_BETWEEN_COORDS(camCoord.x, camCoord.y, camCoord.z, x, y, z, true);

	const auto angle    = 2 * ATAN2(size, 2 * distance);
	const auto textSize = angle / fov;

	DrawScreenText(text, { screenPos.x, screenPos.y }, textSize, textColor, true, ScreenTextAdjust::Center, {screenPos.x - textSize * maxWidth, screenPos.x + textSize * maxWidth});
}

inline void DrawTextAbovePedHead(const std::string &text, Ped ped, float size, Color color, float distance = 0.4f, float maxWidth = 10000.f)
{
	Vector3 coords = GET_PED_BONE_COORDS(ped, 31086, distance, 0, 0);
	DrawScreenText(text, color, coords.x, coords.y, coords.z, size, maxWidth);
}