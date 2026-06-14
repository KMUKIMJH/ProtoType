/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Logger.cpp
Project:    CS230 Engine
Author:     Jonathan Holmes, Minchan Cho
Created:    March 6, 2025
Updated:    2025-03-06
*/

#include <iostream>
#include <iomanip>
#include "Logger.h"

namespace CS230
{
    Logger::Logger(Logger::Severity severity, bool use_console, std::chrono::system_clock::time_point start)
        : min_level(severity), write_to_console(use_console), start_time(start)
    {
        write_to_console = true;
    }

    double Logger::seconds_since_start()
    {
        return std::chrono::duration<double>(std::chrono::system_clock::now() - start_time).count();
    }

    Logger::~Logger() { }

    void Logger::log(Logger::Severity severity, const std::string& message)
    {
        if (severity < min_level)
        {
            return;
        }

        static constexpr const char* level_names[] = { "Verbose", "Debug", "Event", "Error" };
        const int idx = static_cast<int>(severity);

        std::ostringstream line;
        line.setf(std::ios::fixed);
        line << '[' << std::setprecision(4) << seconds_since_start() << "]\t" << level_names[idx] << "\t" << message << '\n';
        const std::string out = line.str();

        if (write_to_console)
        {
            std::cout << out;
        }
    }
}
