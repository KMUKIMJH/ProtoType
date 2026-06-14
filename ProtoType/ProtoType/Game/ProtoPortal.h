#pragma once
#include "../Engine/GameObject.h"
#include "../Engine/Collision.h"
#include "../Engine/Sprite.h"
#include "../Game/GameObjectTypes.h"
#include "../Engine/Matrix.h"

class ProtoPortal : public CS230::GameObject
{
public:
    enum class PortalState {
        Closed,
        Activating,
        Deactivating
    };

    ProtoPortal(Math::rect worldRect, int targetStageIndex);

    GameObjectTypes Type() override { return GameObjectTypes::Portal; }
    std::string TypeName() override { return "ProtoPortal"; }

    const Math::rect& WorldRect() const { return rect; }
    int TargetIndex() const { return targetIndex; }
    void SetTargetIndex(int idx) { targetIndex = idx; }

    void SetEnabled(bool e) { enabled = e; }
    bool IsEnabled() const { return enabled; }

    void Update(double dt) override;
    void Draw(Math::TransformationMatrix camera_matrix) override;

private:
    Math::rect rect;
    int targetIndex{ 0 };
    bool enabled{ true };

    PortalState currentState;
    double deactivateTimer;
};