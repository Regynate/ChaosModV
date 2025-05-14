#pragma once

#include "scripthookv/inc/types.h"

namespace Hooks
{
	void AddTranslationVector(const Entity entity, const Vector3 &translation);
	void AddRotationVector(const Entity entity, const Vector3 &translation);
	void AddScaleVector(const Entity entity, const Vector3 &translation);
}