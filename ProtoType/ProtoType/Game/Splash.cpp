/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Splash.cpp
Project:    CS230 Engine
Author:     Minchan Cho
Created:    March 11, 2023
*/

#include "../Engine/Engine.h"
#include "../Engine/Input.h" 
#include "States.h"
#include "Splash.h"

Splash::Splash() {}

void Splash::Load() 
{
    counter = 0;
    skipToMenu = false;
    texture = Engine::GetTextureManager().Load("Assets/image/Splash/DigiPen.png");
}

void Splash::Draw() 
{
    Engine::GetWindow().Clear(0x00000000u);
    texture->Draw(Math::TranslationMatrix({ (Engine::GetWindow().GetSize() - texture->GetSize()) / 2.0 }));
}

void Splash::Update(double dt) 
{
    Engine::GetLogger().LogDebug(std::to_string(counter));

    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Space) ||  Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Enter)) 
    {
        skipToMenu = true;
        Engine::GetGameStateManager().ClearNextGameState();
    }
    else if (counter >= 2.0) 
    {
        Engine::GetGameStateManager().ClearNextGameState();
    }

    counter += dt;
}

void Splash::Unload() 
{
    if (skipToMenu) 
    {
        Engine::GetGameStateManager().SetNextGameState(static_cast<int>(States::Menu));
    }
    else
    {
        Engine::GetGameStateManager().SetNextGameState(static_cast<int>(States::Logo));
    }
}
