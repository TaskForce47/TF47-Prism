#include "ticketsystem.h"

#include "api_connector.h"

game_value handle_cmd_ticketsystem_register_vehicle(game_state& gs, game_value_parameter left_args, game_value_parameter right_args)
{
	object vehicle = left_args;
	int cost = right_args;
	
	if (! intercept::sqf::is_kind_of(vehicle, "LandVehicle"))
	{
		return false;
	}

	if (cost < 0)
		cost = cost * -1;


	tf47::prism::ticketsystem::RegisteredVehicle registered_vehicle;
	registered_vehicle.vehicle_type = tf47::prism::helper::get_display_name_vehicle(vehicle);
	registered_vehicle.ticket_cost = cost;
	registered_vehicle.vehicle_object = vehicle;

	const auto killed_eventhandler = intercept::client::addMPEventHandler<intercept::client::eventhandlers_mp::MPKilled>(vehicle, [](object unit, object killer, object instigator, bool use_effects)
		{
			tf47::prism::ticketsystem::handle_vehicle_killed(unit);
		});

	const auto get_in_eventhandler = intercept::client::addEventHandler<intercept::client::eventhandlers_object::GetIn>(vehicle, [](object vehicle, intercept::client::get_in_position position, object unit, rv_turret_path turret_path)
		{
			auto commander = intercept::sqf::commander(unit);
			if (commander.is_null())
			{
				commander = intercept::sqf::effective_commander(unit);
			}

			if (intercept::sqf::is_player(commander))
			{
				r_string player_uid = intercept::sqf::get_player_uid(commander);
				intercept::sqf::set_variable(vehicle, "TF47TicketsystemLastOwner", player_uid);
			}
		});

	registered_vehicle.eventhandlers.push_back(killed_eventhandler);
	tf47::prism::ticketsystem::registered_vehicles.push_back(registered_vehicle);

	intercept::sqf::set_variable(vehicle, "TF47TicketsystemInUse", true, true);
	intercept::sqf::set_variable(vehicle, "TF47TicketsystemCost", cost * -1, true);

	return true;
}

game_value handle_cmd_ticketsystem_register_unit(game_state& gs, game_value_parameter left_args, game_value_parameter right_args)
{
	const std::string slot_name = left_args;

	if (tf47::prism::ticketsystem::registered_slots.count(slot_name) == 0)
	{
		write_log("Cannot find the slot name in the pre generated list of all playable slots", tf47::prism::logger::Error);
		gs.set_script_error(game_state::game_evaluator::evaluator_error_type::foreign, r_string("slot does not exist"));
		return false;
	}

	const r_string slot_type = right_args[0];
	int cost = right_args[1];
	if (cost < 0)
		cost = cost * -1;

	tf47::prism::ticketsystem::RegisteredSlot registered_slot;
	registered_slot.cost = cost;
	registered_slot.slot_name = slot_name;
	registered_slot.slot_type = slot_type;
	
	tf47::prism::ticketsystem::registered_slots.at(slot_name) = registered_slot;

	std::stringstream ss;
	ss << "Slot " << slot_name << " has been registered with a cost of " << cost << " tickets.";
	tf47::prism::logger::write_log(ss.str());
	return true;
}

game_value handle_cmd_ticketsystem_unit_created(game_state& gs, game_value_parameter right_args)
{
	tf47::prism::ticketsystem::handle_unit_created(right_args);
	return true;
}

void tf47::prism::ticketsystem::handle_vehicle_killed(object vehicle)
{
	if (!intercept::sqf::get_variable(vehicle, "TF47TicketsystemInUse", false)) return;

	bool flag = false;
	RegisteredVehicle* saved_registered_vehicle = nullptr;
	for (auto registered_vehicle : registered_vehicles)
	{
		if (! registered_vehicle.vehicle_object.is_null() && registered_vehicle.vehicle_object == vehicle)
		{
			saved_registered_vehicle = &registered_vehicle;
			flag = true;
			return;
		}
	}

	if (!flag) return;
	
	std::stringstream ss;
	ss << saved_registered_vehicle->vehicle_type << " has been destroyed!";
	const r_string message = ss.str();

	const r_string player_uid = intercept::sqf::get_variable(vehicle, "TF47TicketsytemLastOwner", "");
	change_ticket_count(message, saved_registered_vehicle->ticket_cost, player_uid);
}

void tf47::prism::ticketsystem::handle_unit_killed(object unit)
{
	if (! intercept::sqf::get_variable(unit, "TF47TicketsystemInUse", false)) return;

	std::string slot_name = intercept::sqf::str(unit);
	if (registered_units.count(slot_name) == 0) return;

	const auto registered_unit = registered_units.at(slot_name);

	std::stringstream ss;
	ss << "A " << registered_unit.slot_type << " has been killed!";
	const r_string message = ss.str();
	
	const r_string player_uid = intercept::sqf::get_player_uid(unit);
	change_ticket_count(message, registered_unit.ticket_cost, player_uid);
}

void tf47::prism::ticketsystem::handle_unit_created(object unit)
{
	std::string slot_name = intercept::sqf::str(unit);
	if (registered_slots.count(slot_name) == 0) return;

	const auto slot = registered_slots.at(slot_name);
	
	RegisteredUnit registered_unit;
	registered_unit.unit = unit;
	registered_unit.ticket_cost = slot.cost;
	registered_unit.slot_type = slot.slot_type;

	const auto killed_eventhandler = intercept::client::addMPEventHandler<intercept::client::eventhandlers_mp::MPKilled>(unit, [](object unit, object killer, object instigator, bool use_effects)
		{
			handle_unit_killed(unit);
		});

	registered_unit.eventhandlers.push_back(killed_eventhandler);
	registered_units.insert_or_assign(slot_name, registered_unit);
	intercept::sqf::set_variable(unit, "TF47TicketsystemInUse", true, true);
}

void tf47::prism::ticketsystem::display_ticket_change(r_string message, int ticket_change, int new_ticket_count)
{
	std::stringstream ss;

	if (ticket_change == 0) return;
	if (ticket_change > 0)
		ss << message << " - " << ticket_change;
	else
		ss << message << " + " << ticket_change;

	if (!tf47::prism::configuration::advanced_notifications) {

		const auto params_hint = auto_array({ game_value(ss.str()) });
		intercept::sqf::remote_exec(params_hint, "hint", -2, false);
	}
	else
	{
		if (ticket_change > 0) {

			const auto params_notification = auto_array({ r_string("TF47NotificationTicketGain"), { message } });
			intercept::sqf::remote_exec(params_notification, "BIS_fnc_showNotification", -2, false);
		}
		else
		{
			const auto params_notification = auto_array({ r_string("TF47NotificationTicketLoss"), { message } });
			intercept::sqf::remote_exec(params_notification, "BIS_fnc_showNotification", -2, false);
		}
	}
}

void tf47::prism::ticketsystem::change_ticket_count(r_string message, int ticket_change, r_string player_uid)
{
	current_tickets = current_tickets + ticket_change;
	
	intercept::sqf::set_variable(intercept::sqf::mission_namespace(), "TF47TicketsystemTicketCount", current_tickets);
	
	display_ticket_change(message, ticket_change, current_tickets);

	std::thread([message, ticket_change, player_uid]()
		{
			api_connector::ApiClient client;
			client.update_ticket_count(player_uid.c_str(), message.c_str(), ticket_change, current_tickets);
	}).detach();
}

void tf47::prism::ticketsystem::initialize_commands()
{
	auto static cmd_ticketsystem_register_vehicle = intercept::client::host::register_sqf_command(
		"TF47TicketsystemRegisterVehicle", "", handle_cmd_ticketsystem_register_vehicle, game_data_type::BOOL,
		game_data_type::OBJECT, game_data_type::SCALAR);
	auto static cmd_ticketsystem_register_unit = intercept::client::host::register_sqf_command(
		"TF47TicketsystemRegisterUnit", "", handle_cmd_ticketsystem_register_unit,
		game_data_type::BOOL, game_data_type::STRING, game_data_type::ARRAY
	);
	auto static cmd_ticketsystem_handle_unit_created = intercept::client::host::register_sqf_command(
		"TF47TicketsystemUnitCreated", "", handle_cmd_ticketsystem_unit_created,
		game_data_type::BOOL, game_data_type::OBJECT
	);
}

void tf47::prism::ticketsystem::start_ticketsystem()
{
	if (configuration::use_ticketsystem) return;

	logger::write_log("Starting Ticketsystem...");
	
	auto allUnits = intercept::sqf::playable_units();
	for (auto& unit : allUnits)
	{
		
		std::string slot_name = intercept::sqf::str(unit);
		RegisteredSlot registered_slot;
		registered_slot.slot_name = r_string(slot_name);
		registered_slot.cost = 0;
		
		registered_slots.insert_or_assign(slot_name, registered_slot);
	}

	__SQF(
		[
			"CAManBase",
			"initPost",
			{
				TF47TicketsystemUnitCreated (_this select 0);
			},
			true,
			[],
			true
		] call CBA_fnc_addClassEventHandler;
	);
}
