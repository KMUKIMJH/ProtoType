/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Logo.h
Project:    CS230 Engine
Author:     Junhwan Kim
Created:    April 1 , 2026
*/

#pragma once
#include "../Engine/GameState.h"
#include "../Engine/Sprite.h"

class Logo : public CS230::GameState {
public:
    Logo();
    void Load() override;
    void Update(double dt) override;
    void Unload() override;
    void Draw() override;

    std::string GetName() override { return "Logo"; }
private:
    double counter = 0;
    CS230::Sprite* logoSprite;
};
