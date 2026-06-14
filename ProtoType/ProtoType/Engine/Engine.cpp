/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Engine.cpp
Project:    CS230 Engine
Author:     Minchan Cho
Created:    March 11, 2025
*/
#include "Engine.h"
#include "../OpenGL/Viewport.h"
#include "../Stage/StageManager.h"
#include "TextureManager.h"

Engine::Engine() :last_tick(std::chrono::system_clock::now()),
#ifdef _DEBUG
logger(CS230::Logger::Severity::Debug, true, last_tick)
#else
logger(CS230::Logger::Severity::Event, false, last_tick)
#endif
{

}

void Engine::Start(std::string window_title) 
{
    logger.LogEvent("Engine Started");
    window.Start(window_title);
    //Start other services
    renderer.Init();
    last_test = last_tick;
    unsigned int seed = static_cast<unsigned int>(time(NULL));
    srand(seed);
}

void Engine::Stop() 
{
    //Stop all services
    renderer.Shutdown();
    logger.LogEvent("Engine Stopped");
}

void Engine::Update() {
    auto now = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_time = now - last_tick;
    double dt = elapsed_time.count();

    if (dt > 1.0 / TargetFPS)
    {
        last_tick = now;

        logger.LogVerbose("Engine Update");
        frame_count++;
        if (frame_count >= FPSTargetFrames)
        {
            std::chrono::duration<double> actual_time = now - last_test;
            double actual_time_sec = actual_time.count();
            double fps = frame_count / actual_time_sec;
            logger.LogDebug("FPS: " + std::to_string(fps));
            frame_count = 0;
            last_test = now;
        }

        logger.LogVerbose("Engine Update");

        if (dt > 0.05) {
            dt = 0.05;
        }

        gamestatemanager.Update(dt);

        Math::TransformationMatrix projection = GAME200::build_viewport_matrix(window.GetSize());
        renderer.BeginScene(projection);
        gamestatemanager.Draw();
        renderer.EndScene();

        input.Update();
        window.Update();
    }
}

bool Engine::HasGameEnded()
{
    if (Engine::GetGameStateManager().HasGameEnded() || window.IsClosed())
    {
        return  true;
    }
    return false;
};

void Engine::AddFont(const std::filesystem::path& file_name)
{
    fonts.push_back(CS230::Font(file_name));
}
