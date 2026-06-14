/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  GameObjectManager.h
Project:    CS230 Engine
Author:     Minchan Cho
Created:    April 21, 2025
*/

#pragma once
#include <vector>
#include "GameObject.h"
#include "Matrix.h"
#include "Component.h"
#include <list>

namespace Math { class TransformationMatrix; }

namespace CS230 {
    class GameObjectManager : public Component {
    public:
        void Add(GameObject* object);
        void Unload();
        void CollisionTest();
        void UpdateAll(double dt);
        void DrawAll(Math::TransformationMatrix camera_matrix);
        // Expose list of managed objects for iteration (read-only)
        const std::list<GameObject*>& GetObjects() const { return objects; }
    private:
        std::list<GameObject*> objects;
    };
}
