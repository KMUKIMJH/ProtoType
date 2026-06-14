/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  GameObject.h
Project:    CS230 Engine
Author:     Minchan Cho
Created:    April 21, 2025
*/

#pragma once
#include "Sprite.h"
#include "ComponentManager.h"
#include "Collision.h"
#include "ShowCollision.h"

namespace Math { class TransformationMatrix; }

enum class GameObjectTypes;

namespace CS230 {

    class Component;

    class GameObject {
        friend class Sprite;
    public:
        GameObject(Math::vec2 position);
        GameObject(Math::vec2 position, double rotation, Math::vec2 scale);
        virtual ~GameObject() {}

        virtual GameObjectTypes Type() = 0;
        virtual std::string TypeName() = 0;
        bool IsCollidingWith(GameObject* other_object);
        bool IsCollidingWith(Math::vec2 point);
        virtual bool CanCollideWith([[maybe_unused]] GameObjectTypes other_object_type);
        virtual void ResolveCollision([[maybe_unused]] GameObject* other_object) {};

        virtual void Update(double dt);
        virtual void Draw(Math::TransformationMatrix camera_matrix);

        const Math::TransformationMatrix& GetMatrix();
        const Math::vec2& GetPosition() const;
        const Math::vec2& GetVelocity() const;
        const Math::vec2& GetScale() const;
        double GetRotation() const;

        void Destroy() { destroy = true; }
        bool Destroyed() { return destroy; }

        class State {
        public:
            virtual void Enter(GameObject* object) = 0;
            virtual void Update(GameObject* object, double dt) = 0;
            virtual void CheckExit(GameObject* object) = 0;
            virtual void Exit([[maybe_unused]] GameObject* object) {}
            virtual std::string GetName() = 0;
        };
        State* current_state;
        void change_state(State* new_state);

        void SetPosition(Math::vec2 new_position);

        template<typename T>
        T* GetGOComponent() {
            return componentmanager.GetComponent<T>();
        }
    protected:
        void UpdatePosition(Math::vec2 delta);
        void SetVelocity(Math::vec2 new_velocity);
        void UpdateVelocity(Math::vec2 delta);
        void SetScale(Math::vec2 new_scale);
        void UpdateScale(Math::vec2 delta);
        void SetRotation(double new_rotation);
        void UpdateRotation(double delta);

        //Sprite sprite;



        void AddGOComponent(Component* component) {
            componentmanager.AddComponent(component);
        }
        template<typename T>
        void RemoveGOComponent() {
            componentmanager.RemoveComponent<T>();
        }
        void ClearGOComponents() {
            componentmanager.Clear();
        }
        void UpdateGOComponents(double dt) {
            componentmanager.UpdateAll(dt);
        }

    private:
        Math::TransformationMatrix object_matrix;

        double rotation;
        Math::vec2 scale;
        Math::vec2 position;
        Math::vec2 velocity;

        bool Matrix_outdated;
        bool destroy;

        class State_None : public State {
        public:
            void Enter(GameObject*) override {}
            void Update(GameObject*, double) override {}
            void CheckExit(GameObject*) override {}
            std::string GetName() { return ""; }
        };
        State_None state_none;

        ComponentManager componentmanager;
    };


}
