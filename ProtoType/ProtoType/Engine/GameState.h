/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  GameState.h
Project:    CS230 Engine
Author:     Minchan Cho
Created:    March 11, 2025
*/

#pragma once
#include <string>
#include "Componentmanager.h"

namespace CS230 {
    class GameState {
    public:
        virtual void Load() = 0;
        virtual void Update(double dt) = 0;
        virtual void Unload() = 0;
        virtual void Draw() = 0;
        virtual std::string GetName() = 0;

        template<typename T>
        T* GetGSComponent() {
            return componentmanager.GetComponent<T>();
        }
    protected:
        void AddGSComponent(Component* component) {
            componentmanager.AddComponent(component);
        }
        void UpdateGSComponents(double dt) {
            componentmanager.UpdateAll(dt);
        }
        template<typename T>
        void RemoveGSComponent() {
            componentmanager.RemoveComponent<T>();
        }
        void ClearGSComponents() {
            componentmanager.Clear();
        }
    private:
        ComponentManager componentmanager;
    };
}
