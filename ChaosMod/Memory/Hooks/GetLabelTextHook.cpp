#include <stdafx.h>

#include "GetLabelTextHook.h"
#include "Memory/Hooks/Hook.h"

#include <queue>

#define MAX_CUSTOM_LABELS 1000

static std::unordered_map<Hash, std::string> ms_CustomLabels;
static std::queue<Hash> ms_CustomLabelHashes;
static std::unordered_map<int, std::function<std::string(std::string_view)>> ms_ProcessMethods;
static int ms_LastMethodID = 1;
struct LabelInfo
{
	std::string m_Label;
	int m_MethodIDs;
};
static std::unordered_map<Hash, LabelInfo> ms_ProcessedLabels;
static std::mutex ms_GetLabelMutex;

const char *(*OG_GetLabelText)(void *, Hash);
const char *HK_GetLabelText(void *text, Hash hash)
{
	std::lock_guard lock(ms_GetLabelMutex);

	int methodIDs = 0;
	for (const auto &[id, _] : ms_ProcessMethods)
		methodIDs += id;

	if (methodIDs > 0 && ms_ProcessedLabels.contains(hash) && ms_ProcessedLabels[hash].m_MethodIDs == methodIDs)
		return ms_ProcessedLabels[hash].m_Label.data();

	std::string outText;
	const auto &result = ms_CustomLabels.find(hash);
	if (result != ms_CustomLabels.end())
		outText = result->second;
	else
	{
		const char *ogText = OG_GetLabelText(text, hash);
		if (!ogText || methodIDs == 0)
			return ogText;
		outText = ogText;
	}

	for (const auto &[_, process] : ms_ProcessMethods)
		outText = process(outText);

	ms_ProcessedLabels[hash] = { outText, methodIDs };

	return ms_ProcessedLabels[hash].m_Label.data();
}

static bool OnHook()
{
	auto handle =
	    Memory::FindPattern("48 8B CB 8B D0 E8 ? ? ? ? 48 85 C0 0F 95 C0", "48 83 EC 28 E8 ? ? ? ? 48 85 C0 75");
	if (!handle.IsValid())
		return false;

	Memory::AddHook(handle.At(IsLegacy() ? 5 : 4).Into().Get<void>(), HK_GetLabelText, &OG_GetLabelText);

	return true;
}

static void OnCleanup()
{
	std::lock_guard lock(ms_GetLabelMutex);
	Hooks::ClearCustomLabels();
	ms_ProcessMethods.clear();
	ms_ProcessedLabels.clear();
}

static RegisterHook registerHook(OnHook, OnCleanup, "GetLabelText", true);

namespace Hooks
{
	int AddTextProcessMethod(std::function<std::string(std::string_view)> foo)
	{
		std::lock_guard lock(ms_GetLabelMutex);
		const auto id = ms_LastMethodID;
		ms_LastMethodID++;
		ms_ProcessMethods.emplace(id, foo);
		return id;
	}

	void RemoveTextProcessMethod(int id)
	{
		std::lock_guard lock(ms_GetLabelMutex);
		ms_ProcessMethods.erase(id);
	}

	void AddCustomLabel(std::string_view label, const std::string &text)
	{
		auto hash = GET_HASH_KEY(label.data());

		if (ms_CustomLabels.contains(hash))
			return;

		ms_CustomLabels[hash] = text;
		ms_CustomLabelHashes.push(hash);

		if (ms_CustomLabelHashes.size() > MAX_CUSTOM_LABELS)
		{
			auto eraseHash = ms_CustomLabelHashes.front();
			ms_CustomLabels.erase(eraseHash);
			ms_CustomLabelHashes.pop();
		}
	}

	void ClearCustomLabels()
	{
		ms_CustomLabels.clear();
		ms_CustomLabelHashes = std::queue<Hash>();
	}
}