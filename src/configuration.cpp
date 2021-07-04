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

	tf47::prism::logger::write_log("Config.json loaded!");
}