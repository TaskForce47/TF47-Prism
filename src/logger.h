#pragma once

#include "intercept.hpp"
#ifndef TF47_PRISM_LOGGER_H
#define TF47_PRISM_LOGGER_H

#endif //TF47_PRISM_LOGGER_H

namespace tf47::prism::logger
{
    enum LogType
    {
        Information,
        Warning,
        Error
    };

    static r_string to_log_string(const std::string& log_type, const std::string& message) {
        std::stringstream ss;
        ss << "[TF47] (Prism) " << log_type << ": " << message;
        return r_string(ss.str());
    }

    static void write_log(const std::string& message, LogType log_type = LogType::Information)
    {
        intercept::client::invoker_lock thread_lock;
        r_string output;
        if (log_type == Information)
        {
            output = to_log_string("Info", message);
        }
        if (log_type == Error)
        {
            output = to_log_string("Error", message);
        }
        if (log_type == Warning)
        {
            output = to_log_string("Warning", message);
        }
        intercept::sqf::diag_log(output);
    }
}