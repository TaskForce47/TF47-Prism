#pragma once

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
		void create_session(std::string world_name, int mission_id, std::string mission_type);
		void end_session();
		void update_ticket_count(std::string player_uid, std::string message, int ticket_change, int ticket_count_new);
		std::vector<int> get_whitelist(std::string playerUid);
	};
}
