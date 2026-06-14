/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Garvity.h
Project:    CS230 Engine
Author:     Minchan Cho
Created:    May 12, 2025
*/
#pragma once
#include "../Engine/Componentmanager.h"

class Gravity : public CS230::Component
{
public:
    Gravity(double value) : gravity(value) {}
    double GetValue()
    {
        return gravity;
    }
    void SetValue(double value)
    {
        gravity = value;
    }
private:
    double gravity;
};
