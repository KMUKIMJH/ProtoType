/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  ScreenWrap.h

Project:    CS230 Engine
Author:     Minchan Cho
Created:    March 11, 2025
*/
#include "ScreenWrap.h"
#include "../Engine/Engine.h"

void ScreenWrap::Update([[maybe_unused]] double dt) {
    Math::ivec2 frame_size = object.GetGOComponent<CS230::Sprite>()->GetFrameSize();
    if (object.GetPosition().y > Engine::GetWindow().GetSize().y + frame_size.y / 2.0) {
        object.SetPosition({ object.GetPosition().x, 0 - frame_size.y / 2.0 });
    }
    else if (object.GetPosition().y < 0 - frame_size.y / 2.0) {
        object.SetPosition({ object.GetPosition().x, Engine::GetWindow().GetSize().y + frame_size.y / 2.0 });
    }
    if (object.GetPosition().x > Engine::GetWindow().GetSize().x + object.GetGOComponent<CS230::Sprite>()->GetFrameSize().x / 2.0) {
        object.SetPosition({ 0 - frame_size.x / 2.0, object.GetPosition().y });
    }
    else if (object.GetPosition().x < 0 - frame_size.x / 2.0) {
        object.SetPosition({ Engine::GetWindow().GetSize().x + frame_size.x / 2.0, object.GetPosition().y });
    }
}
