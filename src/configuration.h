#pragma once
#include <filesystem>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

#include "intercept.hpp"
#include "../intercept/src/host/common/singleton.hpp"
#include "logger.h"

#define WRITE_LOG_AND_CLEAR_STRINGSTREAM(x) logger::write_log(x.str()); x.clear(); x.str("");

namespace tf47::prism::configuration
{
	class configuration : public intercept::singleton<configuration>
	{
	public:
		std::string api_key;
		std::string hostname;

		int session_id;
		bool session_started;

		int mission_id;
		std::string mission_type;

		void load_configuration();
	};
}