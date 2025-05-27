#include "EntityTracking.h"

#include "Components/EffectDispatcher.h"

Tracking::Tracking()
{
}

void Tracking::OnRun()
{
	for (auto it = m_EntityTrackers.begin(); it != m_EntityTrackers.end();)
	{
		auto &tracker = *it;
		bool keep     = true;
		if (tracker.entity != 0 && !DOES_ENTITY_EXIST(tracker.entity))
			keep = false;
		else if (tracker.tick)
			keep = tracker.tick(tracker.entity, tracker.data);

		if (!keep && tracker.cleanup)
			tracker.cleanup(tracker.data);

		if (keep)
			++it;
		else
			it = m_EntityTrackers.erase(it);
	}

	for (auto it = m_Trackers.begin(); it != m_Trackers.end();)
		if (it->tick && it->tick(it->data))
			++it;
		else
			it = m_Trackers.erase(it);

	for (auto it = m_PersistentTrackers.begin(); it != m_PersistentTrackers.end();)
		if (it->tick && it->tick(it->data))
			++it;
		else
		{
			m_ConfigFile.SetValue(it->configKey, false);
			m_ConfigFile.WriteFile();
			it = m_PersistentTrackers.erase(it);
		}
}

void Tracking::OnModPauseCleanup()
{
	for (auto &tracker : m_EntityTrackers)
		if (tracker.cleanup)
			tracker.cleanup(tracker.data);
	m_EntityTrackers.clear();
}

void Tracking::AddEntityTracker(Entity entity, const std::function<bool(Entity, std::any &)> &tracker,
                                const std::any initData)
{
	m_EntityTrackers.emplace_back(entity, tracker, nullptr, initData);
}

void Tracking::AddEntityTracker(Entity entity, const std::function<bool(Entity)> &tracker)
{
	AddEntityTracker(entity, [tracker](Entity e, std::any) { return tracker(e); }, {});
}

void Tracking::AddEntityCleanupTracker(Entity entity, const std::function<void(std::any &)> &tracker,
                                       const std::any initData)
{
	m_EntityTrackers.emplace_back(entity, nullptr, tracker, initData);
}

void Tracking::AddEntityCleanupTracker(Entity entity, const std::function<void()> &tracker)
{
	AddEntityCleanupTracker(entity, [tracker](std::any) { tracker(); }, {});
}

void Tracking::AddTracker(const std::function<bool(std::any &)> &tracker, const std::any initData,
                          const std::string &configKey)
{
	if (!configKey.empty())
	{
		m_ConfigFile.SetValue(configKey, true);
		m_ConfigFile.WriteFile();
		m_PersistentTrackers.emplace_back(configKey, tracker, initData);
	}
	else
	{
		m_Trackers.emplace_back(tracker, initData);
	}
}

void Tracking::AddTracker(const std::function<bool()> &tracker, const std::string &configKey)
{
	AddTracker([tracker](std::any) { return tracker(); }, configKey);
}

void Tracking::AddTrackerByConfigValue(const std::string configKey, const std::function<bool(std::any &)> &tracker,
                                       const std::any initData)
{
	if (m_ConfigFile.ReadValue<bool>(configKey, false))
		m_PersistentTrackers.emplace_back(configKey, tracker, initData);
}

void Tracking::AddTrackerByConfigValue(const std::string configKey, const std::function<bool()> &tracker)
{
	AddTrackerByConfigValue(configKey, [tracker](std::any) { return tracker(); }, {});
}