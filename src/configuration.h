#pragma once
#include <filesystem>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

#include "intercept.hpp"
#include "logger.h"

namespace tf47::prism::configuration
{
	static std::string api_key;
	static std::string hostname;

	static int session_id = -1;
	static bool session_started = false;

	
	static bool tf47_echelon_loaded = false;
	
	static bool use_whitelist = false;
	static bool use_slot_traits = false;
	static bool use_slot_whitelist = false;
	static bool use_attack_aircraft_whitelist = false;

	static bool use_ticketsystem = false;

	static bool advanced_notifications = false;
	
	static int mission_id = 0;
	static std::string mission_type = "COOP";
	
	inline void load_configuration()
	{
		const std::filesystem::path configFilePath("config.json");

		if (!exists(configFilePath))
			throw std::filesystem::filesystem_error("Config file not found", configFilePath, std::error_code());

		auto stringPath = configFilePath.string();

		std::ifstream i(configFilePath);
		nlohmann::json j;
		i >> j;

		api_key = j["ApiKey"].get<std::string>();
		hostname = j["Hostname"].get<std::string>();
	}

	inline void load_mission_config()
	{
		intercept::client::invoker_lock lock;
		const auto config_entry = intercept::sqf::config_entry(intercept::sqf::mission_config_file()) >> ("TF47Prism");


		if (!is_class(config_entry))
		{
			logger::write_log("Cannot parse mission settings, no config class found in description.ext! Falling back to default settings");
			return;
		}
		use_whitelist = get_number(config_entry >> "UseWhitelist");
		use_slot_traits = get_number(config_entry >> "UseSlotTraits");
		use_slot_whitelist = get_number(config_entry >> "UseSlotWhitelist");
		use_attack_aircraft_whitelist = get_number(config_entry >> "UseAttackAircraftWhitelist");

		use_attack_aircraft_whitelist = get_number(config_entry >> "UseTicketsystem");

		if (is_class(intercept::sqf::config_entry(intercept::sqf::config_file()) >> "CfgPatches" >> "mission_configs"))
		{
			logger::write_log("TF47 Mod CfgNotifcation addon detected! Using advanced notifications!");
			advanced_notifications = true;
		}

		logger::write_log("Mission config settings loaded!");
	}

}
