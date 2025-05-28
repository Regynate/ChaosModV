#pragma once

#include "Memory/Entity.h"

#include "Util/Hash.h"

#include "EntityIterator.h"

#include "Memory/WorldToScreen.h"

const inline std::vector<Hash> treeModels = {
	"prop_bush_lrg_04b"_hash,
	"prop_bush_lrg_04c"_hash,
	"prop_bush_lrg_04d"_hash,
	"prop_desert_iron_01"_hash,
	"prop_fan_palm_01a"_hash,
	"prop_joshua_tree_01a"_hash,
	"prop_joshua_tree_01b"_hash,
	"prop_joshua_tree_01c"_hash,
	"prop_joshua_tree_01d"_hash,
	"prop_joshua_tree_01e"_hash,
	"prop_joshua_tree_02a"_hash,
	"prop_joshua_tree_02b"_hash,
	"prop_joshua_tree_02c"_hash,
	"prop_joshua_tree_02d"_hash,
	"prop_joshua_tree_02e"_hash,
	"prop_palm_fan_02_a"_hash,
	"prop_palm_fan_02_b"_hash,
	"prop_palm_fan_03_a"_hash,
	"prop_palm_fan_03_b"_hash,
	"prop_palm_fan_03_c"_hash,
	"prop_palm_fan_03_c_graff"_hash,
	"prop_palm_fan_03_d"_hash,
	"prop_palm_fan_03_d_graff"_hash,
	"prop_palm_fan_04_a"_hash,
	"prop_palm_fan_04_b"_hash,
	"prop_palm_fan_04_c"_hash,
	"prop_palm_fan_04_d"_hash,
	"prop_palm_huge_01a"_hash,
	"prop_palm_huge_01b"_hash,
	"prop_palm_med_01a"_hash,
	"prop_palm_med_01b"_hash,
	"prop_palm_med_01c"_hash,
	"prop_palm_med_01d"_hash,
	"prop_palm_sm_01a"_hash,
	"prop_palm_sm_01d"_hash,
	"prop_palm_sm_01e"_hash,
	"prop_palm_sm_01f"_hash,
	"prop_rio_del_01"_hash,
	"prop_rio_del_01_l3"_hash,
	"prop_rus_olive"_hash,
	"prop_rus_olive_l2"_hash,
	"prop_rus_olive_wint"_hash,
	"prop_s_pine_dead_01"_hash,
	"prop_tree_birch_01"_hash,
	"prop_tree_birch_02"_hash,
	"prop_tree_birch_03"_hash,
	"prop_tree_birch_03b"_hash,
	"prop_tree_birch_04"_hash,
	"prop_tree_cedar_02"_hash,
	"prop_tree_cedar_03"_hash,
	"prop_tree_cedar_04"_hash,
	"prop_tree_cedar_s_01"_hash,
	"prop_tree_cedar_s_02"_hash,
	"prop_tree_cedar_s_04"_hash,
	"prop_tree_cedar_s_05"_hash,
	"prop_tree_cedar_s_06"_hash,
	"prop_tree_cypress_01"_hash,
	"prop_tree_eng_oak_01"_hash,
	"prop_tree_eng_oak_creator"_hash,
	"prop_tree_eucalip_01"_hash,
	"prop_tree_fallen_01"_hash,
	"prop_tree_fallen_02"_hash,
	"prop_tree_fallen_pine_01"_hash,
	"prop_tree_jacada_01"_hash,
	"prop_tree_jacada_02"_hash,
	"prop_tree_lficus_02"_hash,
	"prop_tree_lficus_03"_hash,
	"prop_tree_lficus_05"_hash,
	"prop_tree_lficus_06"_hash,
	"prop_tree_log_01"_hash,
	"prop_tree_log_02"_hash,
	"prop_tree_maple_02"_hash,
	"prop_tree_maple_03"_hash,
	"prop_tree_mquite_01"_hash,
	"prop_tree_mquite_01_l2"_hash,
	"prop_tree_oak_01"_hash,
	"prop_tree_olive_01"_hash,
	"prop_tree_olive_creator"_hash,
	"prop_tree_pine_01"_hash,
	"prop_tree_pine_02"_hash,
	"prop_tree_stump_01"_hash,
	"prop_w_r_cedar_01"_hash,
	"prop_w_r_cedar_dead"_hash,
	"test_tree_cedar_trunk_001"_hash,
	"test_tree_forest_trunk_01"_hash,
	"test_tree_forest_trunk_04"_hash,
	"test_tree_forest_trunk_base_01"_hash,
	"test_tree_forest_trunk_fall_01"_hash,
};

inline auto GetAllTrees()
{
	std::vector<Entity> arr;

	for (const auto building : GetAllBuildings())
	{
		auto model = Memory::GetEntityModel(building);

		if (model)
		{
			if (std::find(treeModels.begin(), treeModels.end(), model) != treeModels.end())
				arr.push_back(building);
		}
	}

	return arr;
}

inline auto GetAllVisibleBuildings()
{
	std::vector<Entity> arr;

	const auto playerPos = GET_ENTITY_COORDS(PLAYER_PED_ID(), false);

	for (const auto building : GetAllBuildings())
	{
		auto model = Memory::GetEntityModel(building);

		if (model)
		{
			auto coords = GET_ENTITY_COORDS(building, false);
			Vector3 min, max;
			GET_MODEL_DIMENSIONS(model, &min, &max);

			const auto radius =
			    std::max(std::abs(max.x - min.x), std::max(std::abs(max.y - min.y), std::abs(max.z - min.z)));

			if (radius / coords.DistanceTo(playerPos) > 0.02f && IS_SPHERE_VISIBLE(coords.x, coords.y, coords.z, radius))
				arr.push_back(building);
		}
	}

	return arr;
}