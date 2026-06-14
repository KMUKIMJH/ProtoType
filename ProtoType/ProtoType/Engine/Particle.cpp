/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Particle.cpp
Project:    CS230 Engine
Author:     Minchan Cho
Created:    June 3, 2025
*/

#include "Particle.h"

CS230::Particle::Particle(const std::filesystem::path& sprite_file) :
	GameObject({ 0, 0 }), life(0.0)
{
	AddGOComponent(new CS230::Sprite(sprite_file, this));
}

void CS230::Particle::Start(Math::vec2 particle_position, Math::vec2 particle_velocity, double max_life)
{
	SetPosition(particle_position);
	SetVelocity(particle_velocity);
	life = max_life;
	GetGOComponent<CS230::Sprite>()->PlayAnimation(0);
}

void CS230::Particle::Update(double dt)
{
	if (Alive())
	{
		life -= dt;
		SetPosition(GetPosition() + GetVelocity() * dt); 
		GameObject::Update(dt);
	}
}

void CS230::Particle::Draw(Math::TransformationMatrix camera_matrix)
{
	if (Alive())
	{
		GameObject::Draw(camera_matrix);
	}
}