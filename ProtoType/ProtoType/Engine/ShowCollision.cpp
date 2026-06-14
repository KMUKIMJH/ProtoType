/* Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  ShowCollision.cpp
Project:    CS230 Engine
Author:     Minchan Cho
Created:    May 14, 2025
*/
#include "ShowCollision.h"

CS230::ShowCollision::ShowCollision() : enabled(false) {}

void CS230::ShowCollision::Update([[maybe_unused]] double dt) {
    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Tab) == true) {
        enabled = !enabled;
    }
}

bool CS230::ShowCollision::Enabled() {
    return enabled;
}
