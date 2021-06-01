//
// Created by Oliver on 20.05.2021.
//

#include "whitelist.h"

bool tf47::prism::whitelist::do_permission_check(object& unit, object& vehicle)
{
	intercept::client::invoker_lock lock;
	auto vehicle_type = helper::get_object_type(vehicle);
	const r_string player_uid = intercept::sqf::get_player_uid(unit);
	const bool is_driver = intercept::sqf::driver(vehicle) == unit;
	bool is_allowed = true;
	
	std::stringstream ss;
	
	if (vehicle_type == "Helicopter")
	{
		if (is_attack_aircraft(vehicle_type))
		{
			if (configuration::use_slot_traits)
			{
				if (is_driver && !check_trait(unit, TRAIT_HELICOPTER_ATTACK_PILOT))
					kick_from_vehicle(unit, vehicle,
					                  r_string(
						                  "You must be on the rotary wings attack pilot slot to play this vehicle role"));
				if (!is_driver && !check_trait(unit, TRAIT_ATTACK_HELICOPTER_CREW))
					kick_from_vehicle(unit, vehicle,
						r_string(
							"You must be on the rotary wing attack crew slot to play this vehicle role"));
			}
			else
			{
				if (!check_whitelist(unit, std::vector{ WHITELIST_ATTACK_AIRCRAFT, WHITELIST_ROTARY_WING }, {}))
					kick_from_vehicle(unit, vehicle,
						r_string("You are missing permissions to play as an attack rotary wing aircraft"));
			}
		}
	}
	if (vehicle_type == "Plane")
	{
		if (is_attack_aircraft(vehicle_type))
		{
			if (configuration::use_slot_traits)
			{
				if (is_driver && !check_trait(unit, TRAIT_PLANE_PILOT))
					kick_from_vehicle(unit, vehicle,
						r_string(
							"You must be on the fixed wings attack pilot slot to play this vehicle role"));
				if (!is_driver && !check_trait(unit, TRAIT_PLANE_CREW))
					kick_from_vehicle(unit, vehicle,
						r_string(
							"You must be on the fixed wing attack crew slot to play this vehicle role"));
			}
			else
			{
				if (!check_whitelist(unit, std::vector{ WHITELIST_ATTACK_AIRCRAFT, WHITELIST_FIXED_WING }, {}))
					kick_from_vehicle(unit, vehicle,
						r_string("You are missing permissions to play as an attack fixed wing aircraft"));
			}
		}
	}
	if (vehicle_type == "TrackedAPC" || vehicle_type == "Tank" || vehicle_type == "WheeledAPC")
	{
		if (is_attack_aircraft(vehicle_type))
		{
			if (configuration::use_slot_traits)
			{
				if (check_trait(unit, TRAIT_TANK_CREW))
					kick_from_vehicle(unit, vehicle,
						r_string(
							"You must be on the crew slot to play this vehicle role"));
			}
			else
			{
				if (!check_whitelist(unit, std::vector{ WHITELIST_TANK}, {}))
					kick_from_vehicle(unit, vehicle,
						r_string("You are missing permissions to play as a tank"));
			}
		}
	}
}

bool tf47::prism::whitelist::check_whitelist(r_string player_uid, std::vector<int>& required_permissions, std::vector<int>& minimal_permissions)
{
	player_permission_lock.lock();
	if (whitelist::player_permissions.count(player_uid) != 0) 
	{
		auto permissions = player_permissions.at(player_uid);
		
		for each (auto required_permission in required_permissions)
		{
			if (std::find(permissions.begin(), permissions.end(), required_permission) == permissions.end())
				return false;
		}

		for each (auto minimal_permission in minimal_permissions)
		{
			if (std::find(permissions.begin(), permissions.end(), minimal_permission) == permissions.end())
				return true;
		}
		return true;
	}
	else 
	{
		std::stringstream ss;
		ss << "Cannot get whitelist from cached map for player uid: " << player_uid;
		logger::write_log(ss.str(), logger::Warning);
		return false;
	}
	
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

	if (!advanced_notifications) {

		
		const auto params_hint = auto_array({ game_value("You are not allowed to use this vehicle") });

		intercept::sqf::remote_exec(params_hint, "hint", player, false);
		
		
	} else {
		const auto params_notification = auto_array({ r_string("TF47NotificationNotWhitelisted"), { message } });
		intercept::sqf::remote_exec(params_notification, "BIS_fnc_showNotification", player, false);
	}

	const auto params_action = auto_array({ game_value(player) , { game_value("Getout"), game_value(intercept::sqf::vehicle(player)) } });
	intercept::sqf::remote_exec(params_action, "action", player, false);
	return true;
}

bool tf47::prism::whitelist::kick_to_lobby(object player)
{
	intercept::client::invoker_lock lock;

	if (!advanced_notifications) {
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
	return intercept::sqf::is_kind_of(vehicle, "ParachuteBase") || intercept::sqf::speed(vehicle) > 5 || !intercept::sqf::is_touching_ground(vehicle);
}

bool tf47::prism::whitelist::is_attack_aircraft(r_string& classname)
{
	return configuration::use_attack_aircraft_whitelist && std::find(attack_aircraft.begin(), attack_aircraft.end(),
		classname) != attack_aircraft.end();
}

bool tf47::prism::whitelist::init_slot_traits(object& unit)
{
	const r_string slot_name = intercept::sqf::str(unit);
	auto slot = slots.at(slot_name);

	intercept::sqf::set_unit_trait(unit, TRAIT_CCT, false, true);
	intercept::sqf::set_unit_trait(unit, TRAIT_HELICOPTER_ATTACK_PILOT, false, true);
	intercept::sqf::set_unit_trait(unit, TRAIT_HELICOPTER_CREW, false, true);
	intercept::sqf::set_unit_trait(unit, TRAIT_HELICOPTER_PILOT, false, true);
	intercept::sqf::set_unit_trait(unit, TRAIT_PLANE_PILOT, false, true);
	intercept::sqf::set_unit_trait(unit, TRAIT_PLANE_CREW, false, true);
	intercept::sqf::set_unit_trait(unit, TRAIT_TANK_CREW, false, true);
	intercept::sqf::set_unit_trait(unit, TRAIT_UAV, false, true);
	intercept::sqf::set_unit_trait(unit, "TF47_WHITELIST_IS_PLANE_CREW", false, true);

	for (const r_string trait : slot.traits)
	{
		intercept::sqf::set_unit_trait(unit, trait, true, true);
	}
	return true;
}

bool tf47::prism::whitelist::init_player_scripts(object& unit)
{
	
}

bool tf47::prism::whitelist::init_load_whitelist(r_string& player_uid, r_string& player_name)
{
	std::thread([&player_uid, player_name]()
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
	if (player_name.empty()) return false;
	if (player_name.find("HC") > player_name.size()) return false;
	
	std::stringstream ss;
	ss << "Player " << player_name << " connected. Querying player whitelist ...";
	logger::write_log(ss.str());
	init_load_whitelist(player_uid, player_name);
	
	return true;
}

bool tf47::prism::whitelist::reload_whitelist(r_string& player_uid)
{
	std::thread([player_uid]()
		{
			api_connector::ApiClient client;
			auto permissions = client.get_whitelist(player_uid.c_str());
			player_permission_lock.lock();
			player_permissions.insert_or_assign(player_uid, permissions);
			player_permission_lock.unlock();
		}).detach();
}

game_value handle_cmd_register_slot_whitelist(game_state& gs, game_value_parameter left_args, game_value_parameter right_args)
{
	const r_string slot_name = left_args;
	if (tf47::prism::whitelist::slots.count(slot_name) == 0)
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

	std::stringstream ss;
	ss << "Slot whitelist " << slot_name << "has been registered for the slot whitelist";
	tf47::prism::logger::write_log(ss.str());
	return true;
}


game_value handle_cmd_register_slot_traits(game_state& gs, game_value_parameter left_args, game_value_parameter right_args)
{
	const r_string slot_name = left_args;
	if (tf47::prism::whitelist::slots.count(slot_name) == 0)
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

	
	auto slot = tf47::prism::whitelist::slots.at(slot_name);
	slot.traits = traits;
}

game_value handle_cmd_unit_created(game_state& gs, game_value_parameter right_args)
{
	object unit = right_args;

	if (!intercept::sqf::is_player(unit)) return false;
	
	tf47::prism::whitelist::init_player_scripts(unit);
	tf47::prism::whitelist::init_slot_traits(unit);
	return true;
}

game_value handle_cmd_register_attack_aircraft(game_state& gs, game_value_parameter right_args)
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


void tf47::prism::whitelist::initialize_commands()
{
	static auto cmd_unit_created = intercept::client::host::register_sqf_command(
		"TF47WhitelistUnitCreated", "Internal use only", 
		handle_cmd_unit_created,game_data_type::BOOL, game_data_type::OBJECT);
	static auto cmd_register_attack_aircraft = intercept::client::host::register_sqf_command(
		"TF47WhitelistRegisterAttackAircraft", "", 
		handle_cmd_register_attack_aircraft, game_data_type::BOOL, game_data_type::ARRAY);
	static auto cmd_register_slot_whitelist = intercept::client::host::register_sqf_command(
		"TF47WhitelistRegisterSlotWhitelist", "", 
		handle_cmd_register_slot_whitelist, game_data_type::BOOL, game_data_type::STRING, game_data_type::ARRAY);
	static auto cmd_register_slot_traits = intercept::client::host::register_sqf_command(
		"TF47WhitelistRegisterSlotTraits", "", 
		handle_cmd_register_slot_traits, game_data_type::BOOL, game_data_type::STRING, game_data_type::ARRAY);
}



void tf47::prism::whitelist::start_whitelist()
{
	if (!configuration::use_whitelist) return;
	
	if (is_class(intercept::sqf::config_entry(intercept::sqf::config_file()) >> "CfgPatches" >> "mission_configs"))
	{
		logger::write_log("TF47 Mod CfgNotifcation addon detected! Using advanced notifications!");
		advanced_notifications = true;
	}
	

	slots.clear();
	player_permissions.clear();
	
	auto allUnits = intercept::sqf::playable_units();
	for (auto& unit	: allUnits)
	{
		auto slot = Slot();
		slot.slot_name = intercept::sqf::str(unit);
		slot.slot_whitelist_enabled = false;
		slots.insert_or_assign(slot.slot_name, slot);
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

	static auto player_view_changed_event_handler_handle = intercept::client::addMissionEventHandler<intercept::client::eventhandlers_mission::PlayerViewChanged>([](object oldBody, object newBody, object vehicleIn, object oldCameraOn, object newCameraOn, object UAV)
		{
			if (vehicleIn.is_null()) return;
			do_permission_check(newBody, vehicleIn);
		});
}

void tf47::prism::whitelist::start_whitelist_reload()
{
	continue_whitelist_reload_loop = true;
	whitelist_reload_thread = new std::thread([]()
		{
			api_connector::ApiClient client;
			while (continue_whitelist_reload_loop)
			{
				player_permission_lock.lock();
				for (auto [player_uid, permissions] : player_permissions)
				{
					const auto res = client.get_whitelist(player_uid.c_str());
					permissions = res;
				}
				player_permission_lock.unlock();
				std::this_thread::sleep_for(std::chrono::seconds(60));
			}
		});
	whitelist_reload_thread->detach();
}

void tf47::prism::whitelist::stop_whitelist_reload()
{
	continue_whitelist_reload_loop = false;
	whitelist_reload_thread->join();
}
