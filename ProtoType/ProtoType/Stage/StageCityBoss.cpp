#include "StageCityBoss.h"
#include "MapLoader.h"
#include "../Game/CityBoss.h"
#include "StageManager.h"
#include "../Engine/Engine.h"
#include "../Engine/GameObjectManager.h"

void StageCityBoss::Setup(StageManager& stage)
{
    stage.InitStage(1920, 1080);

    CS230::MapLoader::LoadSVG("Assets/Maps/StageBoss.svg", stage);

    auto* boss = new CS230::CityBoss();
    boss->Load({ 1300.0, 80.0 }, 60);

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