#include "whitelist.h"

void tf47::prism::whitelist::do_permission_check(object& unit, object& vehicle)
{
	intercept::client::invoker_lock lock;

	//double checked @Desty :D
	if (!is_kick_allowed(vehicle)) return; //check vehicle is allowed to be checked against the whitelist
	
	
	const bool is_driver = intercept::sqf::is_equal_to(intercept::sqf::driver(vehicle), unit);
	const bool is_commander = intercept::sqf::is_equal_to(intercept::sqf::commander(vehicle),unit);
	const bool is_gunner = intercept::sqf::is_equal_to(intercept::sqf::gunner(vehicle), unit);

	if (!(is_driver || is_commander || is_gunner)) return; //check unit is on a slot that requires a whitelist

	const r_string vehicle_type = helper::get_object_type(vehicle);
	
	std::stringstream ss;
	
	if (vehicle_type == "Helicopter")
	{
		do_permission_check_helicopter(unit, vehicle);
		return;
	}
	if (vehicle_type == "Plane")
	{
		
		do_permission_check_plane(unit, vehicle);
		return;
	}
	if (vehicle_type == "TrackedAPC" || vehicle_type == "Tank" || vehicle_type == "WheeledAPC")
	{
		do_permission_check_tank(unit, vehicle);
	}
}

void tf47::prism::whitelist::do_permission_check_tank(object& unit, object& vehicle)
{
	const r_string player_uid = intercept::sqf::get_player_uid(unit);
	
	if (configuration::configuration::get().use_slot_traits)
	{
		if (!check_trait(unit, TRAIT_TANK_CREW))
		{
			kick_from_vehicle(unit, vehicle, r_string("You need to be on a tank crew slot to use this vehicle"));
		}
	}
	else
	{
		if (!check_whitelist_simple(player_uid, std::vector{WHITELIST_TANK}) || 
			!check_whitelist_simple(player_uid, std::vector{WHITELIST_ADMINISTRATOR, WHITELIST_MODERATOR}))
		{
			kick_from_vehicle(unit, vehicle, r_string("You are not whitelisted for tank vehicles"));
		}
	}
}

void tf47::prism::whitelist::do_permission_check_helicopter(object& unit, object& vehicle)
{
	r_string classname = intercept::sqf::type_of(vehicle);
	const r_string player_uid = intercept::sqf::get_player_uid(unit);
	const bool is_driver = intercept::sqf::driver(unit) == unit;

	if (configuration::configuration::get().use_slot_traits)
	{
		if (is_attack_aircraft(classname)) 
		{
			if (is_driver && !check_trait(unit, TRAIT_HELICOPTER_ATTACK_PILOT))
			{
				kick_from_vehicle(unit, vehicle, r_string("You need to be on a attack helicopter pilot slot to use this vehicle"));
				return;
			}
			if (!check_trait(unit, TRAIT_HELICOPTER_ATTACK_CREW))
			{
				kick_from_vehicle(unit, vehicle, r_string("You need to be on a attack helicopter crew slot to use this vehicle"));
				return;
			}
		}
		else //not attack aircraft
		{
			if (is_driver && !check_trait(unit, TRAIT_HELICOPTER_PILOT)) 
			{
				kick_from_vehicle(unit, vehicle, r_string("You need to be on a helicopter pilot slot to use this vehicle"));
				return;
			} 
			if (!check_trait(unit, TRAIT_HELICOPTER_CREW))
			{
				kick_from_vehicle(unit, vehicle, r_string("You need to be on a helicopter crew slot to use this vehicle"));
				return;
			}
		}	
	}
	else
	{
		if (is_attack_aircraft(classname)) 
		{
			if (!check_whitelist_simple(player_uid, std::vector { WHITELIST_ADMINISTRATOR, WHITELIST_MODERATOR }) || 
				!check_whitelist_strict(player_uid, std::vector { WHITELIST_ROTARY_WING, WHITELIST_ATTACK_AIRCRAFT }))
			{
				kick_from_vehicle(unit, vehicle, r_string("You are not whitelisted for rotory wing attack aircraft"));
				return;
			}
		}
		else 
		{
			if (!check_whitelist_simple(player_uid, std::vector { WHITELIST_ADMINISTRATOR, WHITELIST_MODERATOR, WHITELIST_ROTARY_WING }))
			{
				kick_from_vehicle(unit, vehicle, r_string("You are not whitelisted for rotory wing attack aircraft"));
				return;
			}
		}
	}
}

void tf47::prism::whitelist::do_permission_check_plane(object& unit, object& vehicle)
{
	r_string classname = intercept::sqf::type_of(vehicle);
	const r_string player_uid = intercept::sqf::get_player_uid(unit);
	const bool is_driver = intercept::sqf::driver(unit) == unit;

	if (configuration::configuration::get().use_slot_traits)
	{
		if (is_attack_aircraft(classname))
		{
			if (is_driver && !check_trait(unit, TRAIT_PLANE_ATTACK_PILOT))
			{
				kick_from_vehicle(unit, vehicle, r_string("You need to be on a attack plane pilot slot to use this vehicle"));
				return;
			}
			if (!check_trait(unit, TRAIT_PLANE_ATTACK_CREW))
			{
				kick_from_vehicle(unit, vehicle, r_string("You need to be on a attack plane crew slot to use this vehicle"));
				return;
			}
		}
		else //not attack aircraft
		{
			if (is_driver && !check_trait(unit, TRAIT_PLANE_PILOT))
			{
				kick_from_vehicle(unit, vehicle, r_string("You need to be on a plane pilot slot to use this vehicle"));
				return;
			}
			if (!check_trait(unit, TRAIT_PLANE_CREW))
			{
				kick_from_vehicle(unit, vehicle, r_string("You need to be on a plane crew slot to use this vehicle"));
				return;
			}
		}
	}
	else
	{
		if (is_attack_aircraft(classname))
		{
			if (!check_whitelist_simple(player_uid, std::vector { WHITELIST_ADMINISTRATOR, WHITELIST_MODERATOR }) ||
				!check_whitelist_strict(player_uid, std::vector { WHITELIST_FIXED_WING, WHITELIST_ATTACK_AIRCRAFT }))
			{
				kick_from_vehicle(unit, vehicle, r_string("You are not whitelisted for fixed wing attack aircraft"));
				return;
			}
		}
		else
		{
			if (!check_whitelist_simple(player_uid, std::vector { WHITELIST_ADMINISTRATOR, WHITELIST_MODERATOR, WHITELIST_FIXED_WING }))
			{
				kick_from_vehicle(unit, vehicle, r_string("You are not whitelisted for fixed wing attack aircraft"));
				return;
			}
		}
	}
}

void tf47::prism::whitelist::do_permission_check_uav(object& unit, object& uav)
{
}

bool tf47::prism::whitelist::check_whitelist_strict(r_string player_uid, std::vector<int>& required_permissions)
{
	if (player_permissions.exist(player_uid))
	{
		auto permissions = player_permissions.get(player_uid);
		
		for (auto required_permission : required_permissions)
		{
			if (std::find(permissions->begin(), permissions->end(), required_permission) == permissions->end())
				return false;
		}
	}
	else
	{
		std::stringstream ss;
		ss << "Cannot get whitelist from cached map for player uid: " << player_uid;
		write_log(ss.str(), logger::Warning);
		return false;
	}
	return true;
}

bool tf47::prism::whitelist::check_whitelist_simple(const r_string player_uid, std::vector<int>& minimal_permissions)
{
	if (player_permissions.exist(player_uid))
	{
		auto permissions = player_permissions.get(player_uid);

		for (auto required_permission : minimal_permissions)
		{
			if (std::find(permissions->begin(), permissions->end(), required_permission) != permissions->end())
				return true;
		}
	}
	else
	{
		std::stringstream ss;
		ss << "Cannot get whitelist from cached map for player uid: " << player_uid;
		write_log(ss.str(), logger::Warning);
		return false;
	}
	return false;
}

bool tf47::prism::whitelist::check_trait(object& unit, const std::string required_trait)
{
	auto result = intercept::sqf::get_unit_trait(unit, required_trait);
	return std::get<bool>(result);
}

bool tf47::prism::whitelist::kick_from_vehicle(object& player, object& vehicle, r_string message)
{
	intercept::client::invoker_lock lock;
	if (!is_kick_allowed(player))
		return false;

	if (!configuration::configuration::get().advanced_notifications) 
	{	
		const auto params_hint = auto_array({ game_value("You are not allowed to use this vehicle") });
		intercept::sqf::remote_exec(params_hint, "hint", player, false);
	}
	else 
	{
		const auto params_notification = auto_array({ game_value("TF47NotificationNotWhitelisted"), game_value(auto_array({ game_value(message) } ))});
		intercept::sqf::remote_exec(params_notification, "BIS_fnc_showNotification", player, false);
	}

	const auto params_action = auto_array({ game_value(player) , { game_value("Getout"), game_value(intercept::sqf::vehicle(player)) } });
	intercept::sqf::remote_exec(params_action, "action", player, false);
	return true;
}

bool tf47::prism::whitelist::kick_to_lobby(object player)
{
	intercept::client::invoker_lock lock;

	if (!configuration::configuration::get().advanced_notifications) {
		const auto params = auto_array({ game_value("end1"), game_value("false") });
		intercept::sqf::remote_exec(params, "BIS_fnc_endMission", player, false);
	} else {
		const auto params = auto_array({ game_value("TF47WhitelistedSlot"), game_value("false") });
		intercept::sqf::remote_exec(params, "BIS_fnc_endMission", player, false);
	}
	
	return true;
}

bool tf47::prism::whitelist::is_kick_allowed(object& vehicle)
{
	return !intercept::sqf::is_kind_of(vehicle, "ParachuteBase") || intercept::sqf::speed(vehicle) < 5 || intercept::sqf::is_touching_ground(vehicle);
}

bool tf47::prism::whitelist::is_attack_aircraft(r_string& classname)
{
	return configuration::configuration::get().use_attack_aircraft_whitelist && std::find(attack_aircraft.begin(), attack_aircraft.end(),
		classname) != attack_aircraft.end();
}

bool tf47::prism::whitelist::init_slot_traits(object& unit)
{
	const r_string slot_name = intercept::sqf::str(unit);
	auto slot = slots.get(slot_name);

	intercept::sqf::set_unit_trait(unit, TRAIT_CCT, false, true);
	intercept::sqf::set_unit_trait(unit, TRAIT_HELICOPTER_ATTACK_PILOT, false, true);
	intercept::sqf::set_unit_trait(unit, TRAIT_HELICOPTER_CREW, false, true);
	intercept::sqf::set_unit_trait(unit, TRAIT_HELICOPTER_PILOT, false, true);
	intercept::sqf::set_unit_trait(unit, TRAIT_PLANE_PILOT, false, true);
	intercept::sqf::set_unit_trait(unit, TRAIT_PLANE_CREW, false, true);
	intercept::sqf::set_unit_trait(unit, TRAIT_TANK_CREW, false, true);
	intercept::sqf::set_unit_trait(unit, TRAIT_UAV, false, true);
	intercept::sqf::set_unit_trait(unit, TRAIT_PLANE_ATTACK_PILOT, false, true);
	intercept::sqf::set_unit_trait(unit, TRAIT_PLANE_ATTACK_CREW, false, true);

	for (const r_string trait : slot->traits)
	{
		intercept::sqf::set_unit_trait(unit, trait, true, true);
	}
	return true;
}

bool tf47::prism::whitelist::init_player_scripts(object& unit)
{
	if (configuration::configuration::get().use_slot_traits) return true;
	/*
	__SQF(
		[{
			addMissionEventHandler ["PlayerViewChanged", { [_this] remoteExec ["TF47WhitelistPlayerViewChanged", 2] }];
		}] remoteExec ["call", _this];
	).capture(unit);*/

	__SQF(
		[{
			if (! (player getVariable ["TF47WhitelistEventhandlerInitialized", false])) then {

				player addEventHandler ["GetInMan", {
					[_this] remoteExec ["TF47WhitelistGetInMan", 2];
				}];
				player addEventHandler ["SeatSwitchedMan", {
					[_this] remoteExec ["TF47WhitelistSeatSwitchedMan", 2];
				}];
				player setVariable ["TF47WhitelistEventhandlerInitialized", true];
				diag_log "[TF47_Prism] (Whitelist) Client Eventhandler initialized";
			};
		}] remoteExec["bis_fnc_call", _this];
	).capture(unit);

	/*
	const auto get_in_eventhandler = intercept::client::addEventHandler<intercept::client::eventhandlers_object::GetInMan>(
		unit, [](object vehicle, intercept::client::get_in_position position, object unit, rv_turret_path turret_path)
		{
			do_permission_check(unit, vehicle);
		});
	const auto seat_switched_eventhandler = intercept::client::addEventHandler<
		intercept::client::eventhandlers_object::SeatSwitchedMan>
		(unit, [](object vehicle, object unit1, object unit2)
		{
			do_permission_check(unit1, vehicle);
		});

	unit_whitelist_eventhandler.push_back(get_in_eventhandler);
	unit_whitelist_eventhandler.push_back(seat_switched_eventhandler);*/

	
	return true;
}

bool tf47::prism::whitelist::init_load_whitelist(r_string& player_uid, r_string& player_name)
{
	std::thread([player_uid, player_name]()
		{
			api_connector::ApiClient client;
			if (! client.check_user_exist(player_uid.c_str()))
			{
				client.create_user(player_uid.c_str(), player_name.c_str());
				std::stringstream ss;
				ss << "Created new database profile for user: " << player_name << " with uid:" << player_uid;
				write_log(ss.str(), logger::Information);
			}

			reload_whitelist(player_uid);
		}).detach();
	return true;
}

bool tf47::prism::whitelist::handle_player_connected(r_string& player_uid, r_string& player_name)
{
	if (player_name == "__SERVER__") return false;
	if (player_uid.empty()) return false;
	if (player_name.empty()) return false;
	if (player_name.find("HC") <= player_name.size()) return false;
	
	std::stringstream ss;
	ss << "Player " << player_name << " connected. Querying player whitelist ...";
	logger::write_log(ss.str());
	init_load_whitelist(player_uid, player_name);
	
	return true;
}

bool tf47::prism::whitelist::reload_whitelist(r_string player_uid)
{
	std::thread([player_uid]()
		{
			api_connector::ApiClient client;
			auto permissions = client.get_whitelist(player_uid.c_str());
			player_permissions.insert(std::pair{ player_uid, permissions }, true);
		}).detach();
	return true;
}

game_value handle_cmd_register_slot_whitelist(game_state& gs, game_value_parameter left_args, game_value_parameter right_args)
{
	const r_string slot_name = left_args;
	if (! tf47::prism::whitelist::slots.exist(slot_name))
	{
		write_log("Cannot find the slot name in the pre generated list of all playable slots", tf47::prism::logger::Error);
		gs.set_script_error(game_state::game_evaluator::evaluator_error_type::foreign, r_string("slot does not exist"));
		return false;
	}

	std::vector<int> required_permissions;
	for (auto required_permission: right_args[0].to_array())
	{
		required_permissions.push_back(required_permission);
	}

	std::vector<int> minimal_permissions;
	for (auto minimal_permission : right_args[1].to_array())
	{
		minimal_permissions.push_back(minimal_permission);
	}

	auto slot = tf47::prism::whitelist::slots.get(slot_name);
	slot->minimal_required_permission = minimal_permissions;
	slot->strict_required_permission = required_permissions;
	
	std::stringstream ss;
	ss << "Slot whitelist " << slot_name << "has been registered for the slot whitelist";
	tf47::prism::logger::write_log(ss.str());
	return true;
}


game_value handle_cmd_whitelist_register_slot_traits(game_state& gs, game_value_parameter left_args, game_value_parameter right_args)
{
	const r_string slot_name = left_args;
	if (!tf47::prism::whitelist::slots.exist(slot_name) == 0)
	{
		write_log("Cannot find the slot name in the pre generated list of all playable slots", tf47::prism::logger::Error);
		gs.set_script_error(game_state::game_evaluator::evaluator_error_type::foreign, r_string("slot does not exist"));
		return false;
	}

	std::vector<r_string> traits;
	for (game_value element : right_args.to_array())
	{
		traits.push_back(element);
	}

	auto slot = tf47::prism::whitelist::slots.get(slot_name);
	slot->traits = traits;

	return true;
}

game_value handle_cmd_whitelist_unit_created(game_state& gs, game_value_parameter right_args)
{
	object unit = right_args;

	if (!intercept::sqf::is_player(unit)) return false;
	
	tf47::prism::whitelist::init_player_scripts(unit);
	tf47::prism::whitelist::init_slot_traits(unit);
	return true;
}

game_value handle_cmd_whitelist_register_attack_aircraft(game_state& gs, game_value_parameter right_args)
{
	auto_array<game_value> aircraft;
	try 
	{
		aircraft = right_args.to_array();
	}
	catch (std::exception& ex)
	{
		write_log("Cannot register attack aircraft, make sure u pass them as classnames in an array", tf47::prism::logger::Error);
		gs.set_script_error(game_state::game_evaluator::evaluator_error_type::foreign, r_string("Error in passed arguments"));
		return false;
	}
	for (r_string arg : aircraft)
	{
		tf47::prism::whitelist::attack_aircraft.push_back(arg);
	}

	std::stringstream ss;
	ss << "Registered " << aircraft.count() << " aircraft!";
	tf47::prism::logger::write_log(ss.str());
	
	return true;
}

game_value handle_cmd_whitelist_player_view_changed(game_state& gs, game_value_parameter right_args)
{
	//object oldBody, object newBody, object vehicleIn, object oldCameraOn, object newCameraOn, object UAV
	object old_body = right_args[0];
	object new_body = right_args[1];
	object vehicle_in = right_args[2];
	object old_camera_on = right_args[3];
	object new_camera_on = right_args[4];
	object uav = right_args[5];
	
	tf47::prism::whitelist::do_permission_check(new_body, vehicle_in);
	return true;
}

game_value handle_cmd_whitelist_get_in_man(game_state& gs, game_value_parameter right_args)
{
	object unit = right_args[0];
	object vehicle = right_args[2];

	tf47::prism::whitelist::do_permission_check(unit, vehicle);
	return true;
}

game_value handle_cmd_whitelist_seat_switched_man(game_state& gs, game_value_parameter right_args)
{
	object unit = right_args[0];
	object vehicle = right_args[2];

	tf47::prism::whitelist::do_permission_check(unit, vehicle);
	return true;
}

void tf47::prism::whitelist::initialize_commands()
{
	static auto cmd_whitelist_unit_created = intercept::client::host::register_sqf_command(
		"TF47WhitelistUnitCreated", "Internal use only", 
		handle_cmd_whitelist_unit_created,game_data_type::BOOL, game_data_type::OBJECT);
	static auto cmd_whitelist_register_attack_aircraft = intercept::client::host::register_sqf_command(
		"TF47WhitelistRegisterAttackAircraft", "", 
		handle_cmd_whitelist_register_attack_aircraft, game_data_type::BOOL, game_data_type::ARRAY);
	static auto cmd_whitelist_register_slot_whitelist = intercept::client::host::register_sqf_command(
		"TF47WhitelistRegisterSlotWhitelist", "", 
		handle_cmd_register_slot_whitelist, game_data_type::BOOL, game_data_type::STRING, game_data_type::ARRAY);
	static auto cmd_whitelist_register_slot_traits = intercept::client::host::register_sqf_command(
		"TF47WhitelistRegisterSlotTraits", "", 
		handle_cmd_whitelist_register_slot_traits, game_data_type::BOOL, game_data_type::STRING, game_data_type::ARRAY);
	static auto cmd_whitelist_player_view_changed = intercept::client::host::register_sqf_command(
		"TF47WhitelistPlayerViewChanged", "",
		handle_cmd_whitelist_player_view_changed, game_data_type::BOOL, game_data_type::ARRAY);
	static auto cmd_whitelist_get_in_man = intercept::client::host::register_sqf_command(
		"TF47WhitelistGetInMan", "",
		handle_cmd_whitelist_get_in_man, game_data_type::BOOL, game_data_type::ARRAY);
	static auto cmd_whitelist_seat_switched_man = intercept::client::host::register_sqf_command(
		"TF47WhitelistSeatSwitchedMan", "",
		handle_cmd_whitelist_seat_switched_man, game_data_type::BOOL, game_data_type::ARRAY
	);
}



void tf47::prism::whitelist::start_whitelist()
{
	if (!configuration::configuration::get().use_whitelist) return;

	slots.clear();
	player_permissions.clear();
	
	auto allUnits = intercept::sqf::playable_units();
	for (auto& unit	: allUnits)
	{
		auto slot = Slot();
		slot.slot_name = intercept::sqf::str(unit);
		slot.slot_whitelist_enabled = false;
		slots.insert(std::pair{ slot.slot_name, slot }, true);
	}

	__SQF(
		[
			"CAManBase",
			"initPost",
			{
				TF47WhitelistUnitCreated (_this select 0);
			},
			true,
			[],
			true
		] call CBA_fnc_addClassEventHandler;
	);
	
	static auto connected_event_handler_handle = intercept::client::addMissionEventHandler<intercept::client::eventhandlers_mission::PlayerConnected>([](int id, r_string uid, r_string name, bool jip, int owner)
		{
			handle_player_connected(uid, name);
		});
	/*
	static auto player_view_changed_event_handler_handle = intercept::client::addMissionEventHandler<intercept::client::eventhandlers_mission::PlayerViewChanged>([](object oldBody, object newBody, object vehicleIn, object oldCameraOn, object newCameraOn, object UAV)
		{
			if (vehicleIn.is_null()) return;
			do_permission_check(newBody, vehicleIn);
		});
		*/
	start_whitelist_reload();
}

void tf47::prism::whitelist::start_whitelist_reload()
{
	continue_whitelist_reload_loop = true;
	whitelist_reload_thread = new std::thread([]()
		{
			try 
			{
				api_connector::ApiClient client;

				auto time_last_update = std::chrono::high_resolution_clock::now();

				while (continue_whitelist_reload_loop)
				{

					const auto time_delta = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - time_last_update);
					if (time_delta.count() > 60000) {
						auto permissions = player_permissions.get_vector();
						for (size_t i = 0; i < permissions.size(); i++)
						{
							const auto permissions_response = client.get_whitelist(permissions[i].first.c_str());
							permissions[i].second = permissions_response;
						}
						time_last_update = std::chrono::high_resolution_clock::now();
					}

					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
			}
			catch (...)
			{
				std::cout << "Main whitelist reload thread malfunction...";	
			}
		});
	//whitelist_reload_thread->detach();
}

void tf47::prism::whitelist::stop_whitelist_reload()
{
	continue_whitelist_reload_loop = false;
	if (whitelist_reload_thread->joinable())
		whitelist_reload_thread->join();
}
