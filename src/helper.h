#pragma once

#ifndef TF47_PRISM_HELPER_H
#define TF47_PRISM_HELPER_H

#endif //TF47_PRISM_WHITELIST_H

#include "intercept.hpp"

namespace tf47::prism::helper
{
	static r_string get_object_type(object game_object)
	{
		intercept::client::invoker_lock lock;
		game_value object_type_return;
		__SQF(
			(_this call BIS_fnc_objectType) select 1;
		).capture(game_object, object_type_return);
		return object_type_return;
	}

	static std::string get_display_name_vehicle(object& vehicle)
	{
		const std::string vehicleName = get_text(
			intercept::sqf::config_entry(intercept::sqf::config_file()) >> "CfgVehicles" >>
			intercept::sqf::type_of(vehicle) >> "displayName");
		return vehicleName;
	}
}
