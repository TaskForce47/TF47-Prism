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
}
