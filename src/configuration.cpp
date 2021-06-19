#include "configuration.h"

void tf47::prism::configuration::configuration::load_configuration()
{
	const std::filesystem::path configFilePath("config.json");

	if (!exists(configFilePath))
		throw std::filesystem::filesystem_error("Config file not found! Make sure it is placed in the arma3 main folder.", configFilePath, std::error_code());

	auto stringPath = configFilePath.string();

	logger::write_log("Reading config.json...");
	
	std::ifstream i(configFilePath);
	nlohmann::json j;
	i >> j;

	api_key = j["ApiKey"].get<std::string>();
	hostname = j["Hostname"].get<std::string>();

	logger::write_log("Config.json loaded!");
}

void tf47::prism::configuration::configuration::load_mission_config()
{
	intercept::client::invoker_lock lock;
	const auto config_entry = intercept::sqf::config_entry(intercept::sqf::mission_config_file()) >> ("TF47Prism");

	logger::write_log("reading mission config...");
	

	if (!is_class(config_entry))
	{
		logger::write_log(
			"Cannot parse mission settings, no config class found in description.ext! Falling back to default settings");
		return;
	}

	use_whitelist = get_number(config_entry >> "UseWhitelist");
	use_slot_traits = get_number(config_entry >> "UseSlotTraits");
	use_slot_whitelist = get_number(config_entry >> "UseSlotWhitelist");
	use_attack_aircraft_whitelist = get_number(config_entry >> "UseAttackAircraftWhitelist");
	use_ticketsystem = get_number(config_entry >> "UseTicketsystem");

	mission_id = get_number(config_entry >> "MissionId");
	mission_type = get_text(config_entry >> "MissionType");


	if (is_class(intercept::sqf::config_entry(intercept::sqf::config_file()) >> "CfgPatches" >> "TF47_mission_configs"))
	{
		logger::write_log("TF47 Mod CfgNotifcation addon detected! Using advanced notifications!");
		advanced_notifications = true;
	}

	logger::write_log("Mission config settings loaded!");
}

void tf47::prism::configuration::configuration::log_loaded_settings()
{
	logger::write_log("==== LOADED SETTINGS ====");

	std::stringstream ss;
	ss << "ApiKey: " << configuration::api_key;
	WRITE_LOG_AND_CLEAR_STRINGSTREAM(ss)
		ss << "Hostname: " << configuration::hostname;
	WRITE_LOG_AND_CLEAR_STRINGSTREAM(ss)
		ss << "UseWhitelist: " << configuration::use_whitelist;
	WRITE_LOG_AND_CLEAR_STRINGSTREAM(ss)
		ss << "UseSlotTraits: " << configuration::use_slot_traits;
	WRITE_LOG_AND_CLEAR_STRINGSTREAM(ss)
		ss << "UseSlotWhitelist: " << configuration::use_slot_whitelist;
	WRITE_LOG_AND_CLEAR_STRINGSTREAM(ss)
		ss << "UseAttackAircraftWhitelist: " << configuration::use_attack_aircraft_whitelist;
	WRITE_LOG_AND_CLEAR_STRINGSTREAM(ss)
		ss << "UseTicketsystem: " << configuration::use_ticketsystem;
	WRITE_LOG_AND_CLEAR_STRINGSTREAM(ss)
		ss << "MissionId: " << configuration::mission_id;
	WRITE_LOG_AND_CLEAR_STRINGSTREAM(ss)
		ss << "MissionType: " << configuration::mission_type;
	WRITE_LOG_AND_CLEAR_STRINGSTREAM(ss)

		logger::write_log("=========================");
}
