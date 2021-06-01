//
// Created by Oliver on 20.05.2021.
//
#pragma once

#ifndef TF47_PRISM_API_CONNECTOR_H
#define TF47_PRISM_API_CONNECTOR_H

#endif //TF47_PRISM_API_CONNECTOR_H

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "configuration.h"
#include "logger.h"

using json = nlohmann::json;


namespace tf47::prism::api_connector
{
	class ApiClient
	{
	private:
	public:
		ApiClient();
		~ApiClient();

		bool check_user_exist(std::string player_uid);
		void create_user(std::string player_uid, std::string player_name);
		std::vector<int> get_whitelist(std::string playerUid);
	};
}
