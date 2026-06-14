/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Timer.h
Project:    CS230 Engine
Author:     Minchan Cho
Created:    May 20, 2025
*/
#include "Timer.h"

CS230::Timer::Timer(double time_remaining)
{
	Set(time_remaining);
}

void CS230::Timer::Set(double time_remaining)
{
	timer_max = time_remaining;
	Reset();
	pendulum = false;
}
void CS230::Timer::Update(double dt)
{ 
	if (timer >= dt) {
		timer -= dt;
	}
	else if (timer < dt) {
		timer = 0.0;
	}
	pendulum = !pendulum;
}
void CS230::Timer::Reset()
{
	timer = timer_max;
}
double CS230::Timer::Remaining()
{
	return timer;
}
int CS230::Timer::RemainingInt()
{
	return static_cast<int>(timer);
}
bool CS230::Timer::TickTock()
{
	return pendulum;
}