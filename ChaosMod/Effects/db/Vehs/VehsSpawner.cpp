#include <stdafx.h>

static Vector3 GetPlayerPos()
{
	Ped playerPed = PLAYER_PED_ID();

	return GET_ENTITY_COORDS(playerPed, false);
}

static void OnStartRhino()
{
	Vector3 playerPos = GetPlayerPos();

	CreatePoolVehicle(GET_HASH_KEY("RHINO"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));
}

static RegisterEffect registerEffect1(EFFECT_SPAWN_TANK, OnStartRhino, EffectInfo
	{
		.Name = "Spawn Rhino",
		.Id = "spawn_rhino",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);
static void OnStartAdder()
{
	Vector3 playerPos = GetPlayerPos();

	CreatePoolVehicle(GET_HASH_KEY("ADDER"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));
}

static RegisterEffect registerEffect2(EFFECT_SPAWN_ADDER, OnStartAdder, EffectInfo
	{
		.Name = "Spawn Adder",
		.Id = "spawn_adder",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);
static void OnStartDump()
{
	Vector3 playerPos = GetPlayerPos();

	CreatePoolVehicle(GET_HASH_KEY("DUMP"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));
}

static RegisterEffect registerEffect3(EFFECT_SPAWN_DUMP, OnStartDump, EffectInfo
	{
		.Name = "Spawn Dump",
		.Id = "spawn_dump",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);
static void OnStartMonster()
{
	Vector3 playerPos = GetPlayerPos();

	Vehicle veh = CreatePoolVehicle(GET_HASH_KEY("marshall"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));

	SET_VEHICLE_MOD_KIT(veh, 0);
	int liv = GET_VEHICLE_LIVERY_COUNT(veh);
	SET_VEHICLE_LIVERY(veh, g_Random.GetRandomInt(0, liv - 1));
}

static RegisterEffect registerEffect4(EFFECT_SPAWN_MONSTER, OnStartMonster, EffectInfo
	{
		.Name = "Spawn Monster",
		.Id = "spawn_monster",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);
static void OnStartBMX()
{
	Vector3 playerPos = GetPlayerPos();

	CreatePoolVehicle(GET_HASH_KEY("BMX"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));
}

static RegisterEffect registerEffect5(EFFECT_SPAWN_BMX, OnStartBMX, EffectInfo
	{
		.Name = "Spawn BMX",
		.Id = "spawn_bmx",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);
static void OnStartTug()
{
	Vector3 playerPos = GetPlayerPos();

	CreatePoolVehicle(GET_HASH_KEY("TUG"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));
}

static RegisterEffect registerEffect6(EFFECT_SPAWN_TUG, OnStartTug, EffectInfo
	{
		.Name = "Spawn Tug",
		.Id = "spawn_tug",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);
static void OnStartCargoplane()
{
	Vector3 playerPos = GetPlayerPos();

	CreatePoolVehicle(GET_HASH_KEY("CARGOPLANE"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));
}

static RegisterEffect registerEffect7(EFFECT_SPAWN_CARGO, OnStartCargoplane, EffectInfo
	{
		.Name = "Spawn Cargo Plane",
		.Id = "spawn_cargo",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);
static void OnStartBus()
{
	Vector3 playerPos = GetPlayerPos();

	Vehicle veh = CreatePoolVehicle(GET_HASH_KEY("BUS"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));

	int seats = GET_VEHICLE_MODEL_NUMBER_OF_SEATS(GET_ENTITY_MODEL(veh));

	std::vector<Ped> pedPool;
	for (Ped ped : GetAllPeds())
	{
		if (!IS_PED_A_PLAYER(ped) && IS_PED_HUMAN(ped))
		{
			pedPool.push_back(ped);
		}
	}
	for (int i = -1; i < seats; i++)
	{
		if (pedPool.empty())
		{
			break;
		}
		if (i == -1)
		{
			Ped driver = GET_PED_IN_VEHICLE_SEAT(veh, -1, 0);
			SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(driver, true);
			TASK_VEHICLE_MISSION_PED_TARGET(driver, veh, PLAYER_PED_ID(), 13, 9999.f, 4176732, .0f, .0f, false);
		}
		if (IS_VEHICLE_SEAT_FREE(veh, i, false))
		{
			int randomIndex = g_Random.GetRandomInt(0, pedPool.size() - 1);
			SET_PED_INTO_VEHICLE(pedPool[randomIndex], veh, i);
			pedPool.erase(pedPool.begin() + randomIndex);
		}
	}
}

static RegisterEffect registerEffect8(EFFECT_SPAWN_BUS, OnStartBus, EffectInfo
	{
		.Name = "Spawn Bus",
		.Id = "spawn_bus",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);
static void OnStartBlimp()
{
	Vector3 playerPos = GetPlayerPos();

	CreatePoolVehicle(GET_HASH_KEY("BLIMP"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));
}

static RegisterEffect registerEffect9(EFFECT_SPAWN_BLIMP, OnStartBlimp, EffectInfo
	{
		.Name = "Spawn Blimp",
		.Id = "spawn_blimp",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);
static void OnStartBuzzard()
{
	Vector3 playerPos = GetPlayerPos();

	CreatePoolVehicle(GET_HASH_KEY("BUZZARD"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));
}

static RegisterEffect registerEffect10(EFFECT_SPAWN_BUZZARD, OnStartBuzzard, EffectInfo
	{
		.Name = "Spawn Buzzard",
		.Id = "spawn_buzzard",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);
static void OnStartFaggio()
{
	Vector3 playerPos = GetPlayerPos();

	CreatePoolVehicle(GET_HASH_KEY("FAGGIO"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));
}

static RegisterEffect registerEffect11(EFFECT_SPAWN_FAGGIO, OnStartFaggio, EffectInfo
	{
		.Name = "Spawn Faggio",
		.Id = "spawn_faggio",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);
static void OnStartRuiner3()
{
	Vector3 playerPos = GetPlayerPos();

	CreatePoolVehicle(GET_HASH_KEY("RUINER3"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));
}

static RegisterEffect registerEffect12(EFFECT_SPAWN_RUINER3, OnStartRuiner3, EffectInfo
	{
		.Name = "Spawn Ruined Ruiner",
		.Id = "spawn_ruiner3",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);
static void OnStartRandom()
{
	Vector3 playerPos = GetPlayerPos();

	static std::vector<Hash> vehModels = Memory::GetAllVehModels();

	if (!vehModels.empty())
	{
		Vehicle veh = CreatePoolVehicle(vehModels[g_Random.GetRandomInt(0, vehModels.size() - 1)], playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));

		// Also apply random upgrades
		SET_VEHICLE_MOD_KIT(veh, 0);

		SET_VEHICLE_WHEEL_TYPE(veh, g_Random.GetRandomInt(0, 7));

		for (int i = 0; i < 50; i++)
		{
			int max = GET_NUM_VEHICLE_MODS(veh, i);
			if (max > 0)
			{
				SET_VEHICLE_MOD(veh, i, g_Random.GetRandomInt(0, max - 1), g_Random.GetRandomInt(0, 1));
			}

			TOGGLE_VEHICLE_MOD(veh, i, g_Random.GetRandomInt(0, 1));
		}

		SET_VEHICLE_TYRES_CAN_BURST(veh, g_Random.GetRandomInt(0, 1));
		SET_VEHICLE_WINDOW_TINT(veh, g_Random.GetRandomInt(0, 6));

		SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(veh, g_Random.GetRandomInt(0, 255), g_Random.GetRandomInt(0, 255), g_Random.GetRandomInt(0, 255));
		SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(veh, g_Random.GetRandomInt(0, 255), g_Random.GetRandomInt(0, 255), g_Random.GetRandomInt(0, 255));

		_SET_VEHICLE_NEON_LIGHTS_COLOUR(veh, g_Random.GetRandomInt(0, 255), g_Random.GetRandomInt(0, 255), g_Random.GetRandomInt(0, 255));
		for (int i = 0; i < 4; i++)
		{
			_SET_VEHICLE_NEON_LIGHT_ENABLED(veh, i, true);
		}

		_SET_VEHICLE_XENON_LIGHTS_COLOR(veh, g_Random.GetRandomInt(0, 12));
	}
}

static RegisterEffect registerEffect13(EFFECT_SPAWN_RANDOM, OnStartRandom, EffectInfo
	{
		.Name = "Spawn Random Vehicle",
		.Id = "spawn_random",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);
static void OnStartBaletrailer()
{
	Vector3 playerPos = GetPlayerPos();

	CreatePoolVehicle(GET_HASH_KEY("BALETRAILER"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));
}

static RegisterEffect registerEffect14(EFFECT_SPAWN_BALETRAILER, OnStartBaletrailer, EffectInfo
	{
		.Name = "Spawn Bale Trailer",
		.Id = "spawn_baletrailer",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);
static void OnStartRomero()
{
	Vector3 playerPos = GetPlayerPos();

	CreatePoolVehicle(GET_HASH_KEY("ROMERO"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));
}

static RegisterEffect registerEffect15(EFFECT_SPAWN_ROMERO, OnStartRomero, EffectInfo
	{
		.Name = "Where's The Funeral?",
		.Id = "spawn_romero",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);

static void OnStartWizardBroom()
{
	static const Hash oppressorHash = GET_HASH_KEY("OPPRESSOR2");
	static const Hash broomHash = GET_HASH_KEY("prop_tool_broom");
	LoadModel(oppressorHash);
	LoadModel(broomHash);

	Ped player = PLAYER_PED_ID();
	Vector3 playerPos = GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(player, 0, 1, 0);

	Vehicle veh = CreatePoolVehicle(oppressorHash, playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(player));
	SET_VEHICLE_ENGINE_ON(veh, true, true, false);
	SET_VEHICLE_MOD_KIT(veh, 0);
	for (int i = 0; i < 50; i++)
	{
		int max = GET_NUM_VEHICLE_MODS(veh, i);
		SET_VEHICLE_MOD(veh, i, max > 0 ? max - 1 : 0, false);
	}
	SET_ENTITY_ALPHA(veh, 0, false);
	SET_ENTITY_VISIBLE(veh, false, false);
	
	Object broom = CREATE_OBJECT(broomHash, playerPos.x, playerPos.y + 2, playerPos.z, true, false, false);
	ATTACH_ENTITY_TO_ENTITY(broom, veh, 0, 0, 0, 0.3, -80.0, 0, 0, true, false, false, false, 0, true);
}


static RegisterEffect registerEffect16(EFFECT_VEHS_WIZARD_BROOM, OnStartWizardBroom, nullptr, nullptr, EffectInfo
	{
		.Name = "You're A Wizard, Franklin",
		.Id = "vehs_spawn_wizard_broom",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);

static void OnStartKosatka()
{
	Vector3 playerPos = GetPlayerPos();

	CreatePoolVehicle(GET_HASH_KEY("kosatka"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));
}

static RegisterEffect registerEffect17(EFFECT_SPAWN_KOSATKA, OnStartKosatka, EffectInfo
	{
		.Name = "Spawn Kosatka",
		.Id = "spawn_kosatka",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);
static void OnStartFreight()
{
	Vector3 playerPos = GetPlayerPos();

	CreatePoolVehicle(GET_HASH_KEY("freight"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));
}

static RegisterEffect registerEffect18(EFFECT_SPAWN_FREIGHT, OnStartFreight, EffectInfo
	{
		.Name = "Spawn Freight",
		.Id = "spawn_freight",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);
static void OnStartAKuruma()
{
	Vector3 playerPos = GetPlayerPos();

	Vehicle veh = CreatePoolVehicle(GET_HASH_KEY("kuruma2"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));

	SET_VEHICLE_MOD_KIT(veh, 0);
	for (int i = 0; i < 50; i++)
	{
		int max = GET_NUM_VEHICLE_MODS(veh, i);
		if (max > 0)
		{
			SET_VEHICLE_MOD(veh, i, max - 1, true);
		}

		TOGGLE_VEHICLE_MOD(veh, i, true);
	}

	SET_VEHICLE_TYRES_CAN_BURST(veh, false);
	SET_VEHICLE_WINDOW_TINT(veh, 1);

	SET_VEHICLE_COLOURS(veh, 159, 158);

	_SET_VEHICLE_NEON_LIGHTS_COLOUR(veh, 255, 150, 0);
	for (int i = 0; i < 4; i++)
	{
		_SET_VEHICLE_NEON_LIGHT_ENABLED(veh, i, true);
	}

	_SET_VEHICLE_XENON_LIGHTS_COLOR(veh, 6);
}

static RegisterEffect registerEffect19(EFFECT_SPAWN_ARMORED_KURUMA, OnStartAKuruma, EffectInfo
	{
		.Name = "Spawn Armored Kuruma",
		.Id = "spawn_armored_kuruma",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);
static void OnStartHandler()
{
	Vector3 playerPos = GetPlayerPos();

	CreatePoolVehicle(GET_HASH_KEY("handler"), playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(PLAYER_PED_ID()));
}

static RegisterEffect registerEffect20(EFFECT_SPAWN_HANDLER, OnStartHandler, EffectInfo
	{
		.Name = "Spawn Handler",
		.Id = "spawn_handler",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);

static void OnStartBedmobile()
{
	static const Hash carHash = GET_HASH_KEY("dune");
	static const Hash bedHash = GET_HASH_KEY("apa_mp_h_bed_double_09");
	LoadModel(carHash);
	LoadModel(bedHash);

	Ped player = PLAYER_PED_ID();
	Vector3 playerPos = GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(player, 0, 1, 0);

	Vehicle veh = CreatePoolVehicle(carHash, playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(player));
	SET_VEHICLE_ENGINE_ON(veh, true, true, false);
	SET_VEHICLE_MOD_KIT(veh, 0);
	for (int i = 0; i < 50; i++)
	{
		int max = GET_NUM_VEHICLE_MODS(veh, i);
		SET_VEHICLE_MOD(veh, i, max > 0 ? max - 1 : 0, false);
	}
	SET_ENTITY_ALPHA(veh, 0, false);
	SET_ENTITY_VISIBLE(veh, false, false);

	Object bed = CREATE_OBJECT(bedHash, playerPos.x, playerPos.y + 2, playerPos.z, true, false, false);
	ATTACH_ENTITY_TO_ENTITY(bed, veh, 0, 0, -1.25, -0.65, 0, 0, 180, true, false, false, false, 0, true);
}

static RegisterEffect registerEffect21(EFFECT_VEHS_BEDMOBILE, OnStartBedmobile, nullptr, nullptr, EffectInfo
	{
		.Name = "Spawn Bedmobile",
		.Id = "vehs_spawn_bedmobile",
		.EEffectGroupType = EEffectGroupType::SpawnGeneric
	}
);