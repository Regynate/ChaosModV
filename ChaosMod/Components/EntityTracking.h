#pragma once

#include "Components/Component.h"

#include "../vendor/scripthookv/inc/types.h"

#include "Util/OptionsFile.h"

#include <functional>
#include <string>
#include <vector>

class Tracking : public Component
{
	struct Tracker
	{
		std::function<bool(std::any &)> tick;
		std::any data;

		Tracker(const std::function<bool(std::any &)> &tick, const std::any initData) : tick(tick), data(initData)
		{
		}
	};

	struct EntityTracker
	{
		Entity entity;
		std::function<bool(Entity, std::any &)> tick;
		std::function<void(std::any &)> cleanup;
		std::any data;

		EntityTracker(const Entity entity, const std::function<bool(Entity, std::any &)> &tick,
		              const std::function<void(std::any &)> &cleanup, const std::any initData)
		    : entity(entity), tick(tick), cleanup(cleanup), data(initData)
		{
		}
	};

	struct PersistentTracker
	{
		std::string configKey;
		std::function<bool(std::any &)> tick;
		std::any data;

		PersistentTracker(const std::string configKey, const std::function<bool(std::any &)> &tick,
		                  const std::any initData)
		    : configKey(configKey), tick(tick), data(initData)
		{
		}
	};

	std::vector<Tracker> m_Trackers;
	std::vector<EntityTracker> m_EntityTrackers;
	std::vector<PersistentTracker> m_PersistentTrackers;

	OptionsFile m_ConfigFile { { "chaosmod/configs/tracking.json" } };

  public:
	Tracking();

	virtual void OnRun() override;
	virtual void OnModPauseCleanup() override;

	// gets called every tick; given entity is passed as an argument to tracker
	void AddEntityTracker(Entity entity, const std::function<bool(Entity, std::any &)> &tracker,
	                      const std::any initData);
	void AddEntityTracker(Entity entity, const std::function<bool(Entity)> &tracker);

	// gets called once the entity is removed from the world
	void AddEntityCleanupTracker(Entity entity, const std::function<void(std::any &)> &tracker,
	                             const std::any initData);
	void AddEntityCleanupTracker(Entity entity, const std::function<void()> &tracker);

	// gets called every tick; sets the config key as well
	void AddTracker(const std::function<bool(std::any &)> &tracker, const std::any initData,
	                const std::string &configKey = "");
	void AddTracker(const std::function<bool()> &tracker, const std::string &configKey = "");

	void AddTrackerByConfigValue(const std::string configKey, const std::function<bool(std::any &)> &tracker,
	                             const std::any initData);
	void AddTrackerByConfigValue(const std::string configKey, const std::function<bool()> &tracker);
};