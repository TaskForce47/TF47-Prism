#include "api_connector.h"


tf47::prism::api_connector::ApiClient::ApiClient()
{
}

tf47::prism::api_connector::ApiClient::~ApiClient()
{
}

bool tf47::prism::api_connector::ApiClient::check_user_exist(std::string player_uid)
{
	std::stringstream route;
	route << configuration::hostname << "/api/Player/" << player_uid;

	
	auto response = cpr::Get(cpr::Url{ route.str() }, cpr::Header{
			{ "Content-Type", "application/json" },
			{ "TF47AuthKey", configuration::api_key }
		});

	if (response.status_code != 200)
	{
		std::stringstream ss;
		ss << "Failed to get player from database! [Status code:  " << response.status_code << " ] [Message: " << response.text << " ] [Route: " << route.str() << "]";
		logger::write_log(ss.str(), logger::Warning);
		return false;
	}
}

void tf47::prism::api_connector::ApiClient::create_user(std::string player_uid, std::string player_name)
{
	std::stringstream route;
	route << configuration::hostname << "/api/Player/";

	json j;
	j["PlayerName"] = player_name;
	j["PlayerUid"] = player_uid;

	auto response = cpr::Post(cpr::Url{ route.str() }, cpr::Header{
			{ "Content-Type", "application/json" },
			{ "TF47AuthKey", configuration::api_key }
		}, cpr::Body(j.dump()));

	if (response.status_code != 201)
	{
		std::stringstream ss;
		ss << "Failed to get player from database! [Status code:  " << response.status_code << " ] [Message: " << response.text << " ] [Payload: " << j.dump() << "] [Route: " << route.str() << "]";
		logger::write_log(ss.str(), tf47::prism::logger::Warning);
	}
}

std::vector<int> tf47::prism::api_connector::ApiClient::get_whitelist(std::string playerUid)
{

	std::stringstream route;
	route << configuration::hostname << "/api/whitelist/user/" << playerUid;

	auto response = cpr::Get(cpr::Url(route.str()), cpr::Header{
			{ "Content-Type", "application/json" },
			{ "TF47AuthKey", configuration::api_key }
		});

	if (response.status_code != 200)
	{
		std::stringstream ss;
		ss << "Failed get user permission from the database, statuscode: " << response.status_code << "Body: " << response.text;
		throw ss.str();
	}

	json resJ = json::parse(response.text);
	std::vector<int> whitelists;

	for (const auto& item : resJ["whitelistings"])
	{
		whitelists.push_back(item["whitelistId"]);
	}
	return whitelists;
}