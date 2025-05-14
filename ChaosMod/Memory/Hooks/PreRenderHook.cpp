#include <stdafx.h>

#include "PreRenderHook.h"

#include "Memory/Hooks/Hook.h"
#include "Util/Types.h"

struct Transform
{
	std::vector<ChaosVector3> m_Translation;
	std::vector<ChaosVector3> m_Rotation;
	std::vector<ChaosVector3> m_Scale;

	ChaosMatrix4x4 m_ogMatrix;
	ChaosMatrix4x4 m_ogPhysMatrix;

	Transform() : m_Translation(), m_Rotation(), m_Scale(), m_ogMatrix(), m_ogPhysMatrix()
	{
	}
};

static Transform ms_AllBuildingsTransform;
static std::map<DWORD64, Transform> ms_EntityMap;

static DWORD64 GetMatrixAddr(DWORD64 entityAddr, bool physMatrix)
{
	if (!entityAddr)
		return 0;

	if (physMatrix)
	{
		const auto physInst = *reinterpret_cast<DWORD64 *>(entityAddr + 0x30);
		return physInst ? physInst + 0x20 : 0;
	}
	else
		return entityAddr + 0x60;
}

static DWORD64 AddEntityToMap(DWORD64 entityAddr)
{
	if (!entityAddr)
	{
		return 0;
	}

	auto &entityMap = ms_EntityMap;

	if (entityMap.find(entityAddr) == entityMap.end())
		entityMap.emplace(entityAddr, Transform());
	return entityAddr;
}

static DWORD64 AddEntityToMap(Entity entity)
{
	const auto entityAddr = Memory::GetScriptHandleBaseAddress(entity);
	return AddEntityToMap(entityAddr);
}

static void ApplyTransform(DWORD64 entityAddr, Transform &transform, bool physical)
{
	auto matrixAddr = GetMatrixAddr(entityAddr, physical);

	if (matrixAddr)
	{
		auto matrix4x4 = reinterpret_cast<ChaosMatrix4x4 *>(matrixAddr);
		if (physical)
		{
			transform.m_ogPhysMatrix = *matrix4x4;
		}
		else
		{
			transform.m_ogMatrix = *matrix4x4;
		}

		for (const auto &vector : transform.m_Scale)
			*matrix4x4 = ChaosMatrix4x4::ScaleMatrix(vector) * *matrix4x4;
		for (const auto &vector : transform.m_Rotation)
			*matrix4x4 = ChaosMatrix4x4::RotationMatrix(vector) * *matrix4x4;
		for (const auto &vector : transform.m_Translation)
			*matrix4x4 = ChaosMatrix4x4::TranslationMatrix(vector) * *matrix4x4;
	}
}

static void PreRender(DWORD64 entityAddr, Transform &transform)
{
	ApplyTransform(entityAddr, transform, false);
	ApplyTransform(entityAddr, transform, true);
}

static void PostRender(DWORD64 entityAddr, Transform &transform)
{
	auto matrixAddr = GetMatrixAddr(entityAddr, false);

	if (matrixAddr)
	{
		auto matrix4x4 = reinterpret_cast<ChaosMatrix4x4 *>(matrixAddr);
		*matrix4x4     = transform.m_ogMatrix;
	}

	matrixAddr = GetMatrixAddr(entityAddr, true);

	if (matrixAddr)
	{
		auto matrix4x4 = reinterpret_cast<ChaosMatrix4x4 *>(matrixAddr);
		*matrix4x4     = transform.m_ogPhysMatrix;
	}
}

static void (*OG_CGame__Render)(void);
static void HK_CGame__Render(void)
{
	for (auto &[ped, transform] : ms_EntityMap)
		PreRender(ped, transform);

	OG_CGame__Render();

	for (auto &[ped, transform] : ms_EntityMap)
		PostRender(ped, transform);

	ms_EntityMap.clear();
}

static bool OnHook()
{
	bool res      = true;

	Handle handle = Memory::FindPattern("E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 80 3D ?? ?? ?? ?? ?? 75 0E",
	                                    "E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 0F B6 05 ?? ?? ?? ?? A8 01")
	                    .Into();

	if (!handle.IsValid())
	{
		LOG("Couldn't hook into CGame::Render!");
		return false;
	}

	Memory::AddHook(handle.Get<void>(), HK_CGame__Render, &OG_CGame__Render);

	return res;
}

static void OnCleanup()
{
	ms_EntityMap.clear();
}

void Hooks::AddRotationVector(const Entity entity, const Vector3 &rotation)
{
	if (auto addr = AddEntityToMap(entity))
		ms_EntityMap[addr].m_Rotation.push_back(ChaosVector3(rotation));
}

void Hooks::AddTranslationVector(const Entity entity, const Vector3 &translation)
{
	if (auto addr = AddEntityToMap(entity))
		ms_EntityMap[addr].m_Translation.push_back(ChaosVector3(translation));
}

void Hooks::AddScaleVector(const Entity entity, const Vector3 &scale)
{
	if (auto addr = AddEntityToMap(entity))
		ms_EntityMap[addr].m_Scale.push_back(ChaosVector3(scale));
}

static RegisterHook registerHook(OnHook, OnCleanup, "EntityRender", true);