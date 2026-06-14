/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Particle.h
Project:    CS230 Engine
Author:     Minchan Cho
Created:    June 3, 2025
*/
#pragma once
#include "gameobject.h"
#include "GameObjectManager.h"
#include "../Game/GameObjectTypes.h"

namespace CS230 {
    class Particle : public GameObject {
    public:
        Particle(const std::filesystem::path& sprite_file);
        void Start(Math::vec2 particle_position, Math::vec2 particle_velocity, double max_life);
        void Update(double dt) override;
        void Draw(Math::TransformationMatrix camera_matrix) override;
        bool Alive() {
            return  life > 0;
        }
        GameObjectTypes Type() { return GameObjectTypes::Particle; }
    private:
        double life;
    };

    template<typename T>
    class ParticleManager : public Component {
    public:
        ParticleManager();
        ~ParticleManager();
        void Emit(int count, Math::vec2 emitter_position, Math::vec2 emitter_velocity, Math::vec2 direction, double spread);
    private:
        std::vector<T*> particles;
        int index;
    };

    template<typename T>
    ParticleManager<T>::ParticleManager() :index(0)
    {
        for (int i = 0; i < T::MaxCount; ++i)
        {
            T* particle_ptr = new T();
            particles.push_back(particle_ptr);
            Engine::GetGameStateManager().GetGSComponent<CS230::GameObjectManager>()->Add(particle_ptr);
        }
    }

    template<typename T>
    ParticleManager<T>::~ParticleManager()
    {
        particles.clear();
    }

    template<typename T>
    void ParticleManager<T>::Emit(int count, Math::vec2 emitter_position, Math::vec2 emitter_velocity, Math::vec2 direction, double spread)
    {
        for (int i = 0; i < count; ++i) 
        {
            if (particles[i]->Alive()) 
            {
                Engine::GetLogger().LogDebug("Particle overwritten");
            }

            double angle_variation = 0.0;
            if (spread != 0)
            {
                angle_variation = ((rand() % static_cast<int>(spread * 1024)) / 1024.0f) - spread / 2;
            }
            Math::vec2 random_magnitude = direction * (((rand() % 1024) / 2048.0f) + 0.5f);
            Math::vec2 particle_velocity = Math::RotationMatrix(angle_variation) * random_magnitude + emitter_velocity;
            particles[index]->Start(emitter_position, particle_velocity, T::MaxLife);

            ++index;
            if (index >= particles.size()) 
            {
                index = 0;
            }
        }
    }
}
