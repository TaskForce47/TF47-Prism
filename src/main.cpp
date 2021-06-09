#include <intercept.hpp>

#include <string>
#include <sstream>

#include "logger.h"
#include "whitelist.h"
#include "ticketsystem.h"
#include "configuration.h"
#include "api_connector.h"

using namespace intercept;
using namespace tf47::prism;

int intercept::api_version() {
    return INTERCEPT_SDK_API_VERSION;
}

void intercept::register_interfaces() {}

void kill_mission(std::string error_message)
{
    write_log(error_message, logger::Error);
    sqf::remote_exec(auto_array<game_value> {error_message}, "BIS_fnc_error", 0, false);
    __SQF(
        ["EveryoneLost"] call BIS_fnc_endMissionServer;
    );
}


void intercept::pre_init()
{
	try {
        configuration::load_configuration();   
	}
	catch (std::filesystem::filesystem_error& error)
	{
        write_log(error.what(), logger::Error);
        const std::string error_message{ "Could not parse settings file... Maybe it does not exist?" };
        kill_mission(error_message);
	}
    configuration::load_mission_config();
	sqf::set_variable(sqf::mission_namespace(), "TF47Prism", true);
}

void intercept::pre_start()
{
    if (sqf::get_variable(sqf::mission_namespace(), "TF47Echelon", false))
    {
        configuration::tf47_echelon_loaded = true;
    }
	else
    {
        auto player_connected_eventhandler = intercept::client::addMissionEventHandler<client::eventhandlers_mission::PlayerConnected>([](int id, types::r_string uid, types::r_string name, bool jip, int owner)
            {
                std::thread([name, uid]()
                {
					api_connector::ApiClient client;
                    if (!client.check_user_exist())
                        client.create_user(uid.c_str(), name.c_str());
                	
                }).detach();
            });
    }
	
    whitelist::initialize_commands();
    ticketsystem::initialize_commands();
}

void intercept::post_init()
{
    api_connector::ApiClient client;

    try {
        client.create_session(sqf::world_name().c_str());
    }
	catch (std::exception& ex)
    {
        write_log(ex.what(), logger::Error);
        const std::string error_message{ "Could not create a session... terminating..." };
        kill_mission(error_message);
    }

    sqf::set_variable(sqf::mission_namespace(),"TF47SessionId", configuration::session_id, true);
	
    whitelist::start_whitelist();
    configuration::session_started = true;
}

void intercept::mission_ended()
{
    api_connector::ApiClient client;
    if (configuration::session_started)
    {
        client.end_session();
        configuration::session_started = false;
    }
}

void intercept::handle_unload()
{
    api_connector::ApiClient client;
    if (configuration::session_started)
    {
        client.end_session();
        configuration::session_started = false;
    }
    whitelist::stop_whitelist_reload();
    //whitelist::stop_whitelist_reload();
}