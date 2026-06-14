#pragma once
#include "../Engine/GameObject.h"
#include "../Engine/Rect.h"
#include "../Engine/Vec2.h"
#include "GameObjectTypes.h"
#include <string>

namespace CS230 {
    class Backdoor : public GameObject {
    public:
        enum class Animations {
            Activate = 0,     
            Deactivate = 1,   
            Closed = 2      
        };

        enum class DoorState {
            Locked,         
            Closed,         
            Activating,     
            Deactivating    
        };

        Backdoor(Math::rect r, int idx, std::string nameStr);
        void Update(double dt) override;
        void Draw(Math::TransformationMatrix camM) override;
        GameObjectTypes Type() override;
        std::string TypeName() override;
        const Math::rect& GetRect() const;
        Math::vec2 GetSpawnPosition() const;
        std::string GetName() const;
        int GetIndex() const;

        void SetUIOffset(Math::vec2 offset) { uiOffset = offset; }
        Math::vec2 GetUIOffset() const { return uiOffset; }
        void AddUIOffset(Math::vec2 delta) { uiOffset += delta; }

        void SetActivated(bool active) { isActivated = active; }
        bool IsActivated() const { return isActivated; }

    private:
        Math::rect rect;
        int index;
        std::string name;
        Math::vec2 uiOffset{ 0, 0 };

        bool isActivated{ false };

        DoorState currentState{ DoorState::Locked };
        double deactivateTimer{ 0.0 };
    };
}