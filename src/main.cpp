#include <intercept.hpp>

#include <string>
#include <sstream>

#include "logger.h"
#include "configuration.h"

using namespace intercept;
using namespace tf47::prism;

int intercept::api_version() {
    return INTERCEPT_SDK_API_VERSION;
}

void intercept::register_interfaces() {}

bool echelonLoaded = false;

void intercept::pre_init()
{
    configuration::load_configuration();
	configuration::load_mission_config();
	sqf::set_variable(sqf::mission_namespace(), "TF47Prism", true);
}

void intercept::pre_start()
{
    if (sqf::get_variable(sqf::mission_namespace(), "TF47Echelon", false))
    {
        echelonLoaded = true;
    }
	
    //static auto grad_intercept_template_test_command

}

void intercept::post_init()
{
    
}

void intercept::mission_ended()
{
	
}

void intercept::handle_unload()
{
	
}



