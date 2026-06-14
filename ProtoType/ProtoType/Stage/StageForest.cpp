#include "StageForest.h"
#include "StageManager.h"
#include "MapLoader.h"

void StageForest::Setup(StageManager& stage)
{
    stage.InitStage(8000, 5000);

    CS230::MapLoader::LoadSVG("Assets/Maps/StageForest.svg", stage);

    stage.SetBackdoorUIOffset(1, { -150, 120 });
    stage.SetBackdoorUIOffset(2, { -150, 120 });
    stage.SetBackdoorUIOffset(3, { -150, 120 });
    stage.SetBackdoorUIOffset(4, { -150, 120 });
    stage.SetBackdoorUIOffset(5, { -150, 120 });

    stage.handlers.Setup = nullptr;
    stage.handlers.Update = nullptr;
    stage.handlers.Draw = nullptr;
    stage.handlers.Unload = nullptr;
    CS230::Player* p = stage.GetPlayerPtr();
    if (p)
    {
        p->ResetStageProgress();
    }
}