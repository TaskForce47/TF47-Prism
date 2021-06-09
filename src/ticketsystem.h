#pragma once

#include "intercept.hpp"
#include "configuration.h"
#include "logger.h"
#include "helper.h"

namespace tf47::prism::ticketsystem
{
	static int current_tickets = 0;
	
	struct RegisteredVehicle
	{
		object vehicle_object;
		int ticket_cost;
		r_string vehicle_type;
		std::vector<intercept::client::EHIdentifierHandle> eventhandlers;
	};

	struct RegisteredUnit
	{
		r_string unit;
		int ticket_cost;
		std::vector<intercept::client::EHIdentifierHandle> eventhandlers;
		std::string slot_type;
	};

	struct RegisteredSlot
	{
		r_string slot_name;
		r_string slot_type;
		int cost;
	};
	
	void handle_vehicle_killed(object vehicle);
	void handle_unit_killed(object unit);
	void handle_unit_created(object unit);

	void display_ticket_change(r_string message, int ticket_change, int new_ticket_count);
	void change_ticket_count(r_string message, int ticket_change, r_string player_uid);
	
	static std::unordered_map<std::string, RegisteredSlot> registered_slots;
	static std::vector<RegisteredVehicle> registered_vehicles;
	static std::unordered_map<std::string, RegisteredUnit> registered_units;

	void initialize_commands();
	void start_ticketsystem();
}
