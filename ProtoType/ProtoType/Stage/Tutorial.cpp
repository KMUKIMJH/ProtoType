#include "Tutorial.h"
#include "StageManager.h"
#include "MapLoader.h"

void StageTutorial::Setup(StageManager& stage)
{
    stage.InitStage(6000, 1080);

    CS230::MapLoader::LoadSVG("Assets/Maps/StageTutorial.svg", stage);

    stage.handlers.Setup = nullptr;
    stage.handlers.Update = nullptr;
    stage.handlers.Draw = nullptr;
    stage.handlers.Unload = nullptr;
}