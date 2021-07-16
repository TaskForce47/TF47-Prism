#pragma once

#include <csignal>
#include <intercept.hpp>
#include <string>
#include "logger.h"
#include "helper.h"
#include "configuration.h"
#include "api_connector.h"

using namespace intercept;
using namespace tf47::prism;

helper::thread_safe_queue<std::tuple<code, std::string, std::vector<int>>> permission_query_callback_queue;

void kill_mission(const std::string error_message)
{
    write_log(error_message, logger::Error);
    sqf::remote_exec(auto_array<game_value> {error_message}, "BIS_fnc_error", 0, false);
    __SQF(
        ["EveryoneLost"] call BIS_fnc_endMissionServer;
    );
}

game_value handle_cmd_createPlayer(game_state& gs, game_value_parameter right_args)
{
    std::string player_uid = right_args[0];
    std::string player_name = right_args[1];
	
    std::thread([player_uid, player_name]
        {
            try 
            {
                api_connector::ApiClient client;
                if (!client.check_user_exist(player_uid))
                {
                    client.create_user(player_uid, player_name);
                }
            }
    		catch (...)
            {
                logger::write_log("Failed to create new player");
            }
        }).detach();
	
    return true;
}

game_value handle_cmd_update_ticket_count(game_state& gs, game_value_parameter right_args)
{
    int ticket_change = right_args[0];
    int ticket_count_new = right_args[1];
    std::string message = right_args[2];
    std::string player_uid = right_args[3];
    std::thread([player_uid, message, ticket_change, ticket_count_new]
        {
    		try
    		{
                api_connector::ApiClient client;
                client.update_ticket_count(player_uid, message, ticket_change, ticket_count_new);
    		}
    		catch (...)
    		{
                logger::write_log("Failed to update ticket count");
    		}
        }).detach();

    return true;
}

game_value handle_cmd_get_whitelist(game_state& gs, game_value_parameter right_args)
{
    std::string player_uid = right_args[0];
	const code callback = right_args[1];

	std::thread([player_uid, callback]
		{
            try 
            {
                api_connector::ApiClient client;
                auto permissions = client.get_whitelist(player_uid);
                permission_query_callback_queue.push({ callback, player_uid, permissions });
            }
			catch (...)
            {
                logger::write_log("Failed to update whitelist");
                permission_query_callback_queue.push({ callback, player_uid, std::vector<int>() });
            }
            
        }).detach();

    return true;
}

game_value handle_cmd_create_session(game_state& gs, game_value_parameter right_args)
{
	const int mission_id = right_args[0];
	const std::string mission_type = right_args[1];
    try 
    {
        api_connector::ApiClient client;
        client.create_session(intercept::sqf::world_name(), mission_id, mission_type);
    }
	catch (...)
    {
        kill_mission("Failed to create a session");
    }
    return configuration::configuration::get().session_id;
}

game_value handle_cmd_end_session(game_state& gs)
{
    api_connector::ApiClient client;
    client.end_session();
    return true;
}


int intercept::api_version() {
    return INTERCEPT_SDK_API_VERSION;
}

void intercept::register_interfaces() {}

void intercept::pre_init()
{
    try {
        configuration::configuration::get().load_configuration();
    }
    catch (std::filesystem::filesystem_error& error)
    {
        write_log(error.what(), logger::Error);
        const std::string error_message{ "Could not parse settings file... Maybe it does not exist?" };
        kill_mission(error_message);
    }

    sqf::set_variable(sqf::mission_namespace(), "TF47Prism", true);
}

void intercept::pre_start()
{
    static auto cmd_handle_createPlayer = intercept::client::host::register_sqf_command("TF47PrismCreatePlayer", "", handle_cmd_createPlayer, game_data_type::BOOL, game_data_type::ARRAY);
    static auto cmd_handle_update_ticket_count = intercept::client::host::register_sqf_command("TF47PrismUpdateTicketCount", "", handle_cmd_update_ticket_count, game_data_type::BOOL, game_data_type::ARRAY);
    static auto cmd_handle_get_whitelist = intercept::client::host::register_sqf_command("TF47PrismGetWhitelist", "", handle_cmd_get_whitelist, game_data_type::BOOL, game_data_type::ARRAY);
    static auto cmd_handle_create_session = intercept::client::host::register_sqf_command("TF47PrismCreateSession", "", handle_cmd_create_session, game_data_type::SCALAR, game_data_type::ARRAY);
    static auto cmd_handle_end_session = intercept::client::host::register_sqf_command("TF47PrismEndSession", "", handle_cmd_end_session, game_data_type::BOOL);
}

void intercept::post_init()
{

}

void intercept::mission_ended()
{
    api_connector::ApiClient client;
    if (configuration::configuration::get().session_started)
    {
        client.end_session();
        configuration::configuration::get().session_started = false;
    }
}

void intercept::on_frame()
{
    while(!permission_query_callback_queue.empty())
    {
        auto item = permission_query_callback_queue.pop();
        sqf::call(std::get<0>(item), auto_array({game_value(std::get<1>(item)), game_value(std::get<2>(item))}));
    }
}


void intercept::handle_unload()
{
}