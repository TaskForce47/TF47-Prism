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
	route << configuration::configuration::get().hostname << "/api/Player/" << player_uid;

	
	const auto response = cpr::Get(cpr::Url{ route.str() }, cpr::Header{
			{ "Content-Type", "application/json" },
			{ "TF47AuthKey", configuration::configuration::get().api_key }
		});

	if (response.status_code != 200)
	{
		std::stringstream ss;
		ss << "Failed to get player from database! [Status code:  " << response.status_code << " ] [Message: " << response.text << " ] [Route: " << route.str() << "]";
		logger::write_log(ss.str(), logger::Warning);
		return false;
	}
	return true;
}

void tf47::prism::api_connector::ApiClient::create_user(std::string player_uid, std::string player_name)
{
	std::stringstream route;
	route << configuration::configuration::get().hostname << "/api/Player/";

	json j;
	j["PlayerName"] = player_name;
	j["PlayerUid"] = player_uid;

	const auto response = cpr::Post(cpr::Url{ route.str() }, cpr::Header{
			{ "Content-Type", "application/json" },
			{ "TF47AuthKey", configuration::configuration::get().api_key }
		}, cpr::Body(j.dump()));

	if (response.status_code != 201)
	{
		std::stringstream ss;
		ss << "Failed to get player from database! [Status code:  " << response.status_code << " ] [Message: " << response.text << " ] [Payload: " << j.dump() << "] [Route: " << route.str() << "]";
		logger::write_log(ss.str(), tf47::prism::logger::Warning);
	}
}

void tf47::prism::api_connector::ApiClient::create_session(std::string world_name, int mission_id, std::string mission_type)
{
	json j;
	j["missionId"] = mission_id;
	j["missionType"] = mission_type;
	j["worldName"] = world_name;

	std::stringstream route;
	route << configuration::configuration::get().hostname << "/api/session";


	const auto response = cpr::Post(cpr::Url{ route.str() }, cpr::Header{
			{ "Content-Type", "application/json" },
			{ "TF47AuthKey", configuration::configuration::get().api_key }
		},
		cpr::Body(j.dump()));

	if (response.status_code == 200 || response.status_code == 201)
	{
		json resJ = json::parse(response.text);
		configuration::configuration::get().session_id = resJ["sessionId"].get<int>();
	}
	else 
	{
		std::stringstream ss;
		configuration::configuration::get().session_id = -1;
		ss << "Failed to create new session! [Status code: " << response.status_code << " ] [Message: " << response.text << " ] [Payload: " << j.dump() << "] [Route: " << route.str() << "]";
		throw std::exception(ss.str().c_str());
	}
}

void tf47::prism::api_connector::ApiClient::end_session()
{
	std::stringstream route;
	route << configuration::configuration::get().hostname << "/api/Session/" << configuration::configuration::get().session_id << "/endSession";


	const auto response = cpr::Put(cpr::Url{ route.str() }, cpr::Header{
		                               { "Content-Type", "application/json" },
		                               { "TF47AuthKey", configuration::configuration::get().api_key }
	                               }, cpr::Timeout{ 1000 });

	
	if (response.status_code == 200) 
	{
		
	}
	else 
	{
		std::stringstream ss;
		ss << "Failed to stop session! [Status code: " << response.status_code << " ] [Message: " << response.text << " ] [Route: " << route.str() << "]";
		throw std::exception(ss.str().c_str());
	}
}

void tf47::prism::api_connector::ApiClient::update_ticket_count(std::string player_uid, std::string message,
	int ticket_change, int ticket_count_new)
{	
	std::stringstream route;
	route << configuration::configuration::get().hostname << "/api/ticket/" << configuration::configuration::get().session_id;

	json j;
	j["playerUid"] = player_uid;
	j["ticketChange"] = ticket_change;
	j["ticketCountNew"] = ticket_count_new;
	j["message"] = message;
	
	const auto response = cpr::Post(cpr::Url{ route.str() }, cpr::Header{
			{ "Content-Type", "application/json" },
			{ "TF47AuthKey", configuration::configuration::get().api_key }
		}, cpr::Body(j.dump()));

	if (response.status_code != 200)
	{
		std::stringstream ss;
		ss << "Failed to insert new ticket count to database! [Status code:  " << response.status_code << " ] [Message: " << response.text << " ] [Payload: " << j.dump() << "] [Route: " << route.str() << "]";
		write_log(ss.str(), tf47::prism::logger::Warning);
	}
}

std::vector<int> tf47::prism::api_connector::ApiClient::get_whitelist(std::string player_uid)
{

	std::stringstream route;
	route << configuration::configuration::get().hostname << "/api/whitelist/user/" << player_uid;

	const auto response = cpr::Get(cpr::Url(route.str()), cpr::Header{
			{ "Content-Type", "application/json" },
			{ "TF47AuthKey", configuration::configuration::get().api_key }
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
