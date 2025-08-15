#include <stdafx.h>

#include "PreRenderHook.h"

#include "Memory/Hooks/Hook.h"
#include "Memory/RageSkeleton.h"
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

	Transform(const Transform &other)
	    : m_Translation(other.m_Translation),
	      m_Rotation(other.m_Rotation),
	      m_ogMatrix(other.m_ogMatrix),
	      m_ogPhysMatrix(other.m_ogPhysMatrix)
	{
	}
};

struct EntityBone
{
	Entity entity;
	unsigned int boneId;

	bool operator<(const EntityBone &other) const
	{
		if (entity != other.entity)
			return entity < other.entity;
		return boneId < other.boneId;
	}
};

static std::map<DWORD64, Transform> ms_EntityMap;
static std::map<EntityBone, Transform> ms_EntityBoneMap;
static Vector3 ms_PlayerPos;
static float ms_GroundZ;

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
		return 0;

	auto &entityMap = ms_EntityMap;

	if (!entityMap.contains(entityAddr))
		entityMap.emplace(entityAddr, Transform());
	return entityAddr;
}

static DWORD64 AddEntityToMap(Entity entity)
{
	const auto entityAddr = Memory::GetScriptHandleBaseAddress(entity);
	return AddEntityToMap(entityAddr);
}

static EntityBone AddEntityBoneToBoneMap(Entity entity, unsigned int boneId)
{
	auto &entityMap     = ms_EntityBoneMap;

	const EntityBone eb = { entity, boneId };

	if (!entityMap.contains(eb))
		entityMap.emplace(eb, Transform());
	return eb;
}

static void ApplyTransform(ChaosMatrix4x4 *matrix4x4, Transform &transform)
{
	for (const auto &vector : transform.m_Scale)
		*matrix4x4 = ChaosMatrix4x4::ScaleMatrix(vector) * *matrix4x4;
	for (const auto &vector : transform.m_Rotation)
		*matrix4x4 = ChaosMatrix4x4::RotationMatrix(vector) * *matrix4x4;
	for (const auto &vector : transform.m_Translation)
		matrix4x4->translation = matrix4x4->translation + vector;
}

static void SaveMatrixAndApplyTransform(DWORD64 entityAddr, Transform &transform, bool physical)
{
	auto matrixAddr = GetMatrixAddr(entityAddr, physical);

	if (matrixAddr)
	{
		auto matrix4x4 = reinterpret_cast<ChaosMatrix4x4 *>(matrixAddr);
		if (physical)
			transform.m_ogPhysMatrix = *matrix4x4;
		else
			transform.m_ogMatrix = *matrix4x4;

		ApplyTransform(matrix4x4, transform);
	}
}

static void PreRender(DWORD64 entityAddr, Transform &transform)
{
	SaveMatrixAndApplyTransform(entityAddr, transform, false);
	SaveMatrixAndApplyTransform(entityAddr, transform, true);
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

static void PreRender(Entity entity, unsigned int boneId, Transform &transform)
{
	const auto index = GET_PED_BONE_INDEX(entity, boneId);
	ChaosMatrix4x4 matrix;
	Memory::GetBoneMatrix(entity, index, &matrix);
	transform.m_ogMatrix = matrix;
	ApplyTransform(&matrix, transform);
	Memory::SetBoneMatrix(entity, index, &matrix);
}

static void PostRender(Entity entity, unsigned int boneId, Transform &transform)
{
	const auto index      = GET_PED_BONE_INDEX(entity, boneId);
	ChaosMatrix4x4 matrix = transform.m_ogMatrix;
	Memory::SetBoneMatrix(entity, index, &matrix);
}

static void (*OG_CGame__Render)(void);
static void HK_CGame__Render(void)
{
	for (auto it = ms_EntityMap.begin(); it != ms_EntityMap.end(); ++it)
	{
		auto &[entityAddr, transform] = *it;
		PreRender(entityAddr, transform);
	}

	for (auto it = ms_EntityBoneMap.begin(); it != ms_EntityBoneMap.end(); ++it)
	{
		auto &[entityBone, transform] = *it;
		PreRender(entityBone.entity, entityBone.boneId, transform);
	}

	OG_CGame__Render();

	for (auto it = ms_EntityMap.rbegin(); it != ms_EntityMap.rend(); ++it)
	{
		auto &[entityAddr, transform] = *it;
		PostRender(entityAddr, transform);
	}

	for (auto it = ms_EntityBoneMap.rbegin(); it != ms_EntityBoneMap.rend(); ++it)
	{
		auto &[entityBone, transform] = *it;
		PostRender(entityBone.entity, entityBone.boneId, transform);
	}

	ms_EntityMap.clear();
	ms_EntityBoneMap.clear();
	ms_PlayerPos = GET_ENTITY_COORDS(PLAYER_PED_ID(), false);
	_GET_GROUND_Z_FOR_3D_COORD_2(ms_PlayerPos.x, ms_PlayerPos.y, ms_PlayerPos.z, &ms_GroundZ, false, false);
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
	ms_EntityBoneMap.clear();
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

void Hooks::AddPositionAdjustVector(const Entity entity, const Vector3 &scale)
{
	if (auto addr = AddEntityToMap(entity))
	{
		const auto coords = GET_ENTITY_COORDS(entity, false);
		float height      = coords.z;
		if (Memory::GetEntityType(entity) != EntityType::ENTITY_TYPE_BUILDING)
			height = coords.z - GET_ENTITY_HEIGHT_ABOVE_GROUND(entity);
		ms_EntityMap[addr].m_Translation.push_back(ChaosVector3((coords - ms_PlayerPos).x * (scale.x - 1),
		                                                        (coords - ms_PlayerPos).y * (scale.y - 1),
		                                                        (height - ms_GroundZ) * (scale.z - 1)));
	}
}

void Hooks::AddRotationVector(const Entity entity, unsigned int boneId, const Vector3 &rotation)
{
	auto addr = AddEntityBoneToBoneMap(entity, boneId);
	ms_EntityBoneMap[addr].m_Rotation.push_back(ChaosVector3(rotation));
}

void Hooks::AddTranslationVector(const Entity entity, unsigned int boneId, const Vector3 &translation)
{
	auto addr = AddEntityBoneToBoneMap(entity, boneId);
	ms_EntityBoneMap[addr].m_Translation.push_back(ChaosVector3(translation));
}

void Hooks::AddScaleVector(const Entity entity, unsigned int boneId, const Vector3 &scale)
{
	auto addr = AddEntityBoneToBoneMap(entity, boneId);
	ms_EntityBoneMap[addr].m_Scale.push_back(ChaosVector3(scale));
}

static RegisterHook registerHook(OnHook, OnCleanup, "EntityRender", true);