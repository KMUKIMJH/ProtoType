#pragma once
#include "../Engine/GameObject.h"
#include "../Game/States.h"

namespace CS230
{
    class PlayerState_Idle : public GameObject::State
    {
    public:
        void Enter(CS230::GameObject* o) override;
        void Update(CS230::GameObject* o, double dt) override;
        void CheckExit(CS230::GameObject* o) override;
        std::string GetName() override
        {
            return "Idle";
        }
    };

    class PlayerState_Running : public GameObject::State
    {
    public:
        void Enter(CS230::GameObject* o) override;
        void Update(CS230::GameObject* o, double dt) override;
        void CheckExit(CS230::GameObject* o) override;
        std::string GetName() override
        {
            return "Running";
        }
    };

    class PlayerState_Jumping : public GameObject::State
    {
    public:
        void Enter(CS230::GameObject* o) override;
        void Update(CS230::GameObject* o, double dt) override;
        void CheckExit(CS230::GameObject* o) override;
        std::string GetName() override
        {
            return "Jumping";
        }
    };

    class PlayerState_Falling : public GameObject::State
    {
    public:
        void Enter(CS230::GameObject* o) override;
        void Update(CS230::GameObject* o, double dt) override;
        void CheckExit(CS230::GameObject* o) override;
        std::string GetName() override
        {
            return "Falling";
        }
    };

    class PlayerState_Rolling : public GameObject::State
    {
    public:
        void Enter(CS230::GameObject* o) override;
        void Update(CS230::GameObject* o, double dt) override;
        void CheckExit(CS230::GameObject* o) override;
        void Exit(CS230::GameObject* o) override;
        std::string GetName() override
        {
            return "Rolling";
        }
    };

    class PlayerState_Ziplining : public GameObject::State
    {
    public:
        void Enter(CS230::GameObject* o) override;
        void Update(CS230::GameObject* o, double dt) override;
        void CheckExit(CS230::GameObject* o) override;
        void Exit(CS230::GameObject* o) override;
        std::string GetName() override
        {
            return "Ziplining";
        }
    };
}