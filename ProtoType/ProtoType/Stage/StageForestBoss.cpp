#include "StageForestBoss.h"
#include "StageManager.h"
#include "MapLoader.h"
#include "../Game/ForestBoss.h"
#include "../Engine/Engine.h"

void StageForestBoss::Setup(StageManager& stage)
{
    stage.InitStage(1920, 1080);

    CS230::MapLoader::LoadSVG("Assets/Maps/StageBoss.svg", stage);
    auto* boss = new CS230::ForestBoss();
    boss->Load({ 960.0, 80.0 }, 50);

    GameEntity ge;
    ge.go = boss;
    ge.logic = boss;
    ge.updateFunc = [boss](const CS230::Player& p, double dt, bool s)
        {
            boss->Update(p, dt, s);
        };
    stage.enemies.push_back(ge);

    stage.handlers.Setup = nullptr;
    stage.handlers.Update = nullptr;
    stage.handlers.Draw = nullptr;
    stage.handlers.Unload = nullptr;
}