/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Logger.h
Project:    CS230 Engine
Author:     Jonathan Holmes, Minchan Cho
Created:    March 6, 2025
Updated:    2025-03-06
*/

#pragma once
#include <string>
#include <fstream>
#include <chrono>

namespace CS230
{
    class Logger
    {
    public:
        enum class Severity
        {
            Verbose,  //Minor messages
            Debug,    //Only used while actively debugging
            Event,    //General event, like key press or state change
            Error     //Errors, such as file load errors
        };

        Logger(Severity severity, bool use_console, std::chrono::system_clock::time_point start_time);
        ~Logger();

        void LogError(std::string text)
        {
            log(Severity::Error, std::move(text));
        }
        void LogEvent(std::string text)
        {
            log(Severity::Event, std::move(text));
        }
        void LogDebug(std::string text)
        {
            log(Severity::Debug, std::move(text));
        }
        void LogVerbose(std::string text)
        {
            log(Severity::Verbose, std::move(text));
        }

    private:
        Severity min_level;
        std::ofstream file_stream;
        bool write_to_console = false;
        std::chrono::system_clock::time_point start_time;

        double seconds_since_start();
        void log(Severity severity, const std::string& message);
    };
}
