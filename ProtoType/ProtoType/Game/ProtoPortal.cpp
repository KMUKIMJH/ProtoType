#include "ProtoPortal.h"
#include "../Engine/Engine.h"
#include "../Game/Player.h"
#include "../Stage/StageManager.h"

ProtoPortal::ProtoPortal(Math::rect worldRect, int targetStageIndex)
    : CS230::GameObject({ worldRect.Left(), worldRect.Bottom() })
    , rect(worldRect)
    , targetIndex(targetStageIndex)
    , currentState(PortalState::Closed)
    , deactivateTimer(0.0)
{
    AddGOComponent(new CS230::RectCollision({ {0,0}, { static_cast<int>(rect.Right() - rect.Left()), static_cast<int>(rect.Top() - rect.Bottom()) } }, this));
    
    auto* spr = new CS230::Sprite("Assets/image/Objects/Portal.spt", this);
    AddGOComponent(spr);
    spr->PlayAnimation(2);

    Math::ivec2 fs = spr->GetFrameSize();
    Math::vec2 s{ 1.0, 1.0 };
    if (fs.x > 0)
    {
        s.x = (rect.Right() - rect.Left()) / fs.x;
    }
    if (fs.y > 0)
    {
        s.y = (rect.Top() - rect.Bottom()) / fs.y;
    }
    SetScale(s);
    SetPosition({ rect.Left(), rect.Bottom() });
}

void ProtoPortal::Update(double dt)
{
    UpdateGOComponents(dt);

    if (!enabled) return;

    bool isPlayerClose = false;

    auto* sm = dynamic_cast<StageManager*>(Engine::GetGameStateManager().GetCurrentState());
    if (sm != nullptr)
    {
        CS230::Player* player = sm->GetPlayerPtr(); 
        if (player != nullptr)
        {
            const Math::vec2 portalCenter = { rect.Left() + rect.Size().x * 0.5, rect.Bottom() + rect.Size().y * 0.5 };
            const Math::vec2 playerCenter = player->Center();

            double dx = portalCenter.x - playerCenter.x;
            double dy = portalCenter.y - playerCenter.y;
            double distanceSq = dx * dx + dy * dy;

            const double ActivationDistance = 150.0;
            if (distanceSq < ActivationDistance * ActivationDistance)
            {
                isPlayerClose = true;
            }
        }
    }

    if (auto spr = GetGOComponent<CS230::Sprite>())
    {
        switch (currentState)
        {
        case PortalState::Closed:
            if (isPlayerClose)
            {
                currentState = PortalState::Activating;
                spr->PlayAnimation(0);
            }
            break;

        case PortalState::Activating:
            if (!isPlayerClose)
            {
                currentState = PortalState::Deactivating;
                deactivateTimer = 0.75;
                spr->PlayAnimation(1);
            }
            break;

        case PortalState::Deactivating:
            if (isPlayerClose)
            {
                currentState = PortalState::Activating;
                spr->PlayAnimation(0);
            }
            else
            {
                deactivateTimer -= dt;
                if (deactivateTimer <= 0.0)
                {
                    currentState = PortalState::Closed;
                    spr->PlayAnimation(2);
                }
            }
            break;
        }
    }
}

void ProtoPortal::Draw(Math::TransformationMatrix camera_matrix)
{
    if (auto spr = GetGOComponent<CS230::Sprite>())
    {
        const Math::vec2 size{ rect.Size() };
        const Math::vec2 center{ rect.Left() + size.x * 0.5, rect.Bottom() + size.y * 0.5 };
        Math::ivec2 hs = spr->GetHotSpot(0);
        Math::ivec2 fs = spr->GetFrameSize();
        Math::vec2 adjust{ static_cast<double>(hs.x) - fs.x * 0.5, static_cast<double>(hs.y) - fs.y * 0.5 };
        spr->Draw(camera_matrix * Math::TranslationMatrix(center) * Math::ScaleMatrix(GetScale()) * Math::TranslationMatrix(adjust));
    }
    else
    {
        const Math::vec2 size{ rect.Size() };
        const Math::vec2 center{ rect.Left() + size.x * 0.5, rect.Bottom() + size.y * 0.5 };
        ::Engine::GetRenderer2D().DrawRectangle(camera_matrix * Math::TranslationMatrix(center) * Math::ScaleMatrix(size), GAME200::CLEAR, GAME200::BLUE, 2.0);
    }
}