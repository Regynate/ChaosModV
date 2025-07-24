#pragma once

#include <string_view>
#include <unordered_map>

enum class EffectCategory
{
	None,
	Shader,
	EnhancedShader,
	Timecycle,
	Gravity,
	Pitch,
	Camera,
	TrafficColor,
	Movement
};

inline const std::unordered_map<std::string_view, EffectCategory> g_NameToEffectCategory {
	{ "None", EffectCategory::None },
	{ "Shader", EffectCategory::Shader },
	{ "EnhancedShader", EffectCategory::EnhancedShader },
	{ "Screen", EffectCategory::Timecycle },
	{ "Gravity", EffectCategory::Gravity },
	{ "Pitch", EffectCategory::Pitch },
	{ "Camera", EffectCategory::Camera },
	{ "TrafficColor", EffectCategory::TrafficColor },
	{ "Movement", EffectCategory::Movement },
};