#pragma once

#include "scripthookv/inc/types.h"

namespace Hooks
{
	void AddTranslationVector(const Entity entity, const Vector3 &translation);
	void AddRotationVector(const Entity entity, const Vector3 &rotation);
	void AddScaleVector(const Entity entity, const Vector3 &scale);
	void AddPositionAdjustVector(const Entity entity, const Vector3 &scale);
	void AddTranslationVector(const Entity entity, unsigned int boneId, const Vector3 &translation);
	void AddRotationVector(const Entity entity, unsigned int boneId, const Vector3 &rotation);
	void AddScaleVector(const Entity entity, unsigned int boneId, const Vector3 &scale);
}