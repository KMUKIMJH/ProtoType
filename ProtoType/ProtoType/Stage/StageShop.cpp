#include "StageShop.h"
#include "StageManager.h"
#include "MapLoader.h"
#include "../Game/Generator.h"
#include "../Game/HPStation.h"
#include "../Engine/Engine.h"

void StageShop::Setup(StageManager& stage)
{
    stage.InitStage(1920, 1080);
    CS230::MapLoader::LoadSVG("Assets/Maps/StageShop.svg", stage);

    stage.AddText("[ Basic Reward ]", { 200.0, 500.0 }, 0xFFFFFF00, 1.0);
    stage.AddText("Free HP Station (Interact with E)", { 300.0, 400.0 }, 0xFFFFFFFF, 0.5);

    stage.AddText("[ Achievement ]", { 800.0, 500.0 }, 0xFFFFFF00, 1.0);
    stage.AddText("Reach under 2 mins: 10 Scrap", { 800.0, 450.0 }, 0xFFFFFFFF, 0.5);
    stage.AddText("Kill 7 without hit: 15 Scrap", { 800.0, 400.0 }, 0xFFFFFFFF, 0.5);

    stage.AddText("[ Save Point ]", { 1400.0, 500.0 }, 0xFFFFFF00, 1.0);
    stage.AddText("Dead enemies will not respawn.", { 1400.0, 450.0 }, 0xFFFFFFFF, 0.5);
    stage.AddText("Generator Use Scrap (Interact with E)", { 1300.0, 400.0 }, 0xFFFFFFFF, 0.5);

    CS230::Player* p = stage.GetPlayerPtr();
    if (p)
    {
        if (p->combat.runTimer <= 120.0 && !p->combat.timeRewardClaimed)
        {
            p->AddScrap(10);
            p->combat.timeRewardClaimed = true;
            stage.AddText("Time Bonus: +10 Scrap!", { 800.0, 600.0 }, GAME200::GREEN, 0.8);
        }

        if (p->combat.noHitKillCount >= 7 && !p->combat.noHitRewardClaimed)
        {
            p->AddScrap(15);
            p->combat.noHitRewardClaimed = true;
            stage.AddText("No-Hit Bonus: +15 Scrap!", { 800.0, 650.0 }, GAME200::GREEN, 0.8);
        }
        p->combat.timerPaused = true;
    }

    stage.handlers.Setup = nullptr;
    stage.handlers.Update = nullptr;
    stage.handlers.Draw = nullptr;
    stage.handlers.Unload = nullptr;
}