/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  main.cpp
Project:    CS230 Engine
Author:     Minchan Cho
Created:    March 6, 2025
*/

#include <iostream>
#include "Engine/Engine.h"
#include "Game/Splash.h"
#include "Game/Logo.h"
#include "Game/Menu.h"
#include "Game/Setting.h"
#include "Game/Credits.h"
#include "Game/HowToPlay.h"
#include "Stage/StageManager.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    try {
        Engine& engine = Engine::Instance();
        engine.Start("RAGTAG");
        engine.AddFont("Assets/Fonts/Font_Pixel.png");

        Splash splash;
        engine.GetGameStateManager().AddGameState(splash);
        Logo logo;
        engine.GetGameStateManager().AddGameState(logo);
        Menu menu;
        engine.GetGameStateManager().AddGameState(menu);
        Setting setting;
        engine.GetGameStateManager().AddGameState(setting);
        StageManager stage;
        engine.GetGameStateManager().AddGameState(stage);
        Credits credits;
        engine.GetGameStateManager().AddGameState(credits);
        HowToPlay howToPlay;
        engine.GetGameStateManager().AddGameState(howToPlay);

        while (engine.HasGameEnded() == false) {
            engine.Update();
        }

        engine.Stop();
        return 0;
    }
    catch (std::exception& e) {
        std::cerr << e.what() << "\n";
        return -1;
    }
}
