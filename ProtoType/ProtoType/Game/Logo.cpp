/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Logo.cpp
Project:    CS230 Engine
Author:     Junhwan Kim
Created:    April 1 , 2026
*/

#include "../Engine/Engine.h"
#include "../Engine/Input.h" 
#include "States.h"
#include "Logo.h"

Logo::Logo() {}

void Logo::Load() 
{
    counter = 0;
    logoSprite = new CS230::Sprite("Assets/image/Splash/Logo.spt", nullptr);
    logoSprite->PlayAnimation(0);
}

void Logo::Draw() 
{
    Engine::GetWindow().Clear(0xff000000u);
    Math::ivec2 windowSize = Engine::GetWindow().GetSize();
    Math::ivec2 frameSize = logoSprite->GetFrameSize();
    Math::vec2 translation = { (windowSize.x - frameSize.x) / 2.0, (windowSize.y - frameSize.y) / 2.0 };
    logoSprite->Draw(Math::TranslationMatrix(translation));
}

void Logo::Update(double dt) 
{
    logoSprite->Update(dt);
    Engine::GetLogger().LogDebug(std::to_string(counter));

    bool skip = Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Space) ||  Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Enter);

    if (logoSprite->AnimationEnded() || counter >= 3.8 || skip) 
    {
        Engine::GetGameStateManager().ClearNextGameState();
    }
    counter += dt;
}

void Logo::Unload() 
{
    delete logoSprite;
    logoSprite = nullptr;
    Engine::GetGameStateManager().SetNextGameState(static_cast<int>(States::Menu));
}