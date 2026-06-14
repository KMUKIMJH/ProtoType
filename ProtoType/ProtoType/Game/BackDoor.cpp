#include "Backdoor.h"
#include "Player.h"
#include "../Engine/Engine.h"
#include "../Engine/Sprite.h"
#include "../Stage/StageManager.h"
#include "../OpenGL/RGBA.h"
#include <cmath>

namespace CS230
{
    Backdoor::Backdoor(Math::rect r, int idx, std::string nameStr)
        : GameObject({ r.Left(), r.Bottom() }), rect(r), index(idx), name(nameStr), isActivated(false), currentState(DoorState::Locked), deactivateTimer(0.0)
    {
        auto* spr = new CS230::Sprite("Assets/image/Objects/Back_Door.spt", this);
        AddGOComponent(spr);

        Math::ivec2 fs = spr->GetFrameSize();
        Math::vec2 s{ 1.0, 1.0 };
        if (fs.x > 0) s.x = rect.Size().x / static_cast<double>(fs.x);
        if (fs.y > 0) s.y = rect.Size().y / static_cast<double>(fs.y);
        SetScale(s);

        spr->PlayAnimation(static_cast<int>(Animations::Closed));
    }

    void Backdoor::Update(double dt)
    {
        UpdateGOComponents(dt);

        auto* sm = dynamic_cast<StageManager*>(Engine::GetGameStateManager().GetCurrentState());
        if (sm && sm->GetPlayerPtr())
        {
            Player* player = sm->GetPlayerPtr();

            double centerX = rect.Left() + rect.Size().x * 0.5;
            double distanceX = std::abs(player->Center().x - centerX);
            constexpr double kTriggerRange = 180.0;

            bool isPlayerClose = (distanceX <= kTriggerRange);

            if (auto spr = GetGOComponent<CS230::Sprite>())
            {
                if (!isActivated)
                {
                    currentState = DoorState::Locked;
                    if (spr->CurrentAnimation() != static_cast<int>(Animations::Closed))
                    {
                        spr->PlayAnimation(static_cast<int>(Animations::Closed));
                    }
                }
                else
                {
                    switch (currentState)
                    {
                    case DoorState::Locked:
                        currentState = isPlayerClose ? DoorState::Activating : DoorState::Closed;
                        spr->PlayAnimation(isPlayerClose ? static_cast<int>(Animations::Activate) : static_cast<int>(Animations::Closed));
                        break;

                    case DoorState::Closed:
                        if (isPlayerClose)
                        {
                            currentState = DoorState::Activating;
                            spr->PlayAnimation(static_cast<int>(Animations::Activate));
                        }
                        break;

                    case DoorState::Activating:
                        if (!isPlayerClose)
                        {
                            currentState = DoorState::Deactivating;
                            deactivateTimer = 0.75;
                            spr->PlayAnimation(static_cast<int>(Animations::Deactivate));
                        }
                        break;

                    case DoorState::Deactivating:
                        if (isPlayerClose)
                        {
                            currentState = DoorState::Activating;
                            spr->PlayAnimation(static_cast<int>(Animations::Activate));
                        }
                        else
                        {
                            deactivateTimer -= dt;
                            if (deactivateTimer <= 0.0)
                            {
                                currentState = DoorState::Closed;
                                spr->PlayAnimation(static_cast<int>(Animations::Closed));
                            }
                        }
                        break;
                    }
                }
            }
        }

        SetPosition({ rect.Left(), rect.Bottom() });
    }

    void Backdoor::Draw(Math::TransformationMatrix camM)
    {
        if (auto spr = GetGOComponent<CS230::Sprite>())
        {
            const Math::vec2 size{ rect.Size() };
            const Math::vec2 center{ rect.Left() + size.x * 0.5, rect.Bottom() + size.y * 0.5 };
            Math::ivec2 hs = spr->GetHotSpot(0);
            Math::ivec2 fs = spr->GetFrameSize();
            Math::vec2 adjust{ static_cast<double>(hs.x) - fs.x * 0.5, static_cast<double>(hs.y) - fs.y * 0.5 };

            spr->Draw(camM * Math::TranslationMatrix(center) * Math::ScaleMatrix(GetScale()) * Math::TranslationMatrix(adjust));
        }
        else
        {
            CS230::GameObject::Draw(camM);
        }
    }

    GameObjectTypes Backdoor::Type() { return GameObjectTypes::Backdoor; }
    std::string Backdoor::TypeName() { return "Backdoor"; }
    const Math::rect& Backdoor::GetRect() const { return rect; }
    int Backdoor::GetIndex() const { return index; }
    std::string Backdoor::GetName() const { return name; }

    Math::vec2 Backdoor::GetSpawnPosition() const
    {
        const double playerWidth = 90.0 * 0.8;
        double centerX = rect.Left() + (rect.Right() - rect.Left()) * 0.5;
        return { centerX - playerWidth * 0.5, rect.Bottom() };
    }
}