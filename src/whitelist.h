#pragma once

#ifndef TF47_PRISM_WHITELIST_H
#define TF47_PRISM_WHITELIST_H

#endif //TF47_PRISM_WHITELIST_H

#include "intercept.hpp"
#include "api_connector.h"
#include "configuration.h"
#include "helper.h"
#include "logger.h"

#define WHITELIST_ADMINISTRATOR 1
#define WHITELIST_MODERATOR 2
#define WHITELIST_TF 3
#define WHITELIST_CCT 4
#define WHITELIST_TANK 5
#define WHITELIST_ROTARY_WING 6
#define WHITELIST_FIXED_WING 7
#define WHITELIST_ATTACK_AIRCRAFT 8
#define WHITELIST_UAV 9
#define WHITELIST_BUILDER 10

#define TRAIT_TANK_CREW "TF47_WHITELIST_IS_TANK_CREW"
#define TRAIT_HELICOPTER_PILOT "TF47_WHITELIST_IS_HELICOPTER_PILOT"
#define TRAIT_HELICOPTER_ATTACK_PILOT "TF47_WHITELIST_IS_ATTACK_HELICOPTER_PILOT"
#define TRAIT_UAV "TF47_WHITELIST_IS_UAV"
#define TRAIT_CCT "TF47_WHITELIST_IS_CCT"
#define TRAIT_HELICOPTER_ATTACK_CREW "TF47_WHITELIST_IS_ATTACK_HELICOPTER_CREW"
#define TRAIT_HELICOPTER_CREW "TF47_WHITELIST_IS_HELICOPTER_CREW"
#define TRAIT_PLANE_PILOT "TF47_WHITELIST_IS_PLANE_PILOT"
#define TRAIT_PLANE_CREW "TF47_WHITELIST_IS_PLANE_CREW"
#define TRAIT_PLANE_ATTACK_PILOT "TF47_WHITELIST_IS_ATTACK_PLANE_PILOT"
#define TRAIT_PLANE_ATTACK_CREW "TF47_WHITELIST_IS_ATTACK_PLANE_CREW"

namespace tf47::prism::whitelist
{
	struct Slot
	{
		r_string slot_name;
		std::vector<int> strict_required_permission;
		std::vector<int> minimal_required_permission;
		std::vector<r_string> traits;
		bool slot_whitelist_enabled;
	};

	static bool continue_whitelist_reload_loop;
	static std::thread* whitelist_reload_thread;

	static std::vector<intercept::client::EHIdentifierHandle> unit_whitelist_eventhandler;
	
	static std::vector<r_string> attack_aircraft;
	static helper::thread_safe_vector<r_string, Slot> slots;
	static helper::thread_safe_vector<r_string, std::vector<int>> player_permissions;

	void do_permission_check(object& unit, object& vehicle);
	void do_permission_check_tank(object& unit, object& vehicle);
	void do_permission_check_helicopter(object& unit, object& vehicle);
	void do_permission_check_plane(object& unit, object& vehicle);
	void do_permission_check_uav(object& unit, object& uav);
	
	bool check_whitelist_strict(r_string player_uid, std::vector<int>& required_permissions);
	bool check_whitelist_simple(r_string player_uid, std::vector<int>& minimal_permissions);
	bool check_trait(object& unit, const std::string required_trait);

	bool kick_from_vehicle(object& player, object& vehicle, r_string message);
	bool kick_to_lobby(object player);

	bool is_kick_allowed(object& vehicle);
	bool is_attack_aircraft(r_string& classname);
	
	bool init_slot_traits(object& unit);
	bool init_player_scripts(object& unit);
	bool init_load_whitelist(r_string& player_uid, r_string& player_name);
	
	bool handle_player_connected(r_string& player_uid, r_string& player_name);
	bool reload_whitelist(r_string player_uid);
	
	void initialize_commands();
	void start_whitelist();
	void start_whitelist_reload();
	void stop_whitelist_reload();
}
