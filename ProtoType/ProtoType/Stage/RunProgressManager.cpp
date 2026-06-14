#include "RunProgressManager.h"
#include "../Engine/Engine.h"
#include <algorithm>
#include <random>

namespace CS230
{
    RunProgressManager::RunProgressManager()
    {
        for (int i = 0; i < StageSlotCount; ++i)
        {
            stageOrder[i] = std::min(i, StageSlotCount - 1);
        }
        stageCleared.fill(false);
        enemyAliveStatus.assign(StageSlotCount, std::vector<bool>());
    }

    void RunProgressManager::GenerateStageRun()
    {
        stageOrder[0] = TutorialConfigId;

        struct ThemeBlock
        {
            int stage;
            int shop;
            int boss;
        };

        ThemeBlock factoryTheme = { 0, 6, 3 };

        std::vector<ThemeBlock> remainingThemes = {
            { 1, 6, 4 },
            { 2, 6, 5 }
        };

        auto rng = std::default_random_engine(std::random_device{}());
        std::shuffle(remainingThemes.begin(), remainingThemes.end(), rng);

        stageOrder[1] = factoryTheme.stage;
        stageOrder[2] = factoryTheme.shop;
        stageOrder[3] = factoryTheme.boss;

        stageOrder[4] = remainingThemes[0].stage;
        stageOrder[5] = remainingThemes[0].shop;
        stageOrder[6] = remainingThemes[0].boss;

        stageOrder[7] = remainingThemes[1].stage;
        stageOrder[8] = remainingThemes[1].shop;
        stageOrder[9] = remainingThemes[1].boss;

        stageCleared.fill(false);

        enemyAliveStatus.assign(StageSlotCount, std::vector<bool>());

        stageRunInitialized = true;

        Engine::GetLogger().LogEvent("[RunProgressManager] Stage Order Generated (Factory First)");
    }

    void RunProgressManager::RerollPostMidBoss()
    {}

    void RunProgressManager::ResetRun()
    {
        GenerateStageRun();
    }

    void RunProgressManager::MarkStageCleared(int slot)
    {
        if (slot >= 0 && slot < StageSlotCount)
        {
            stageCleared[slot] = true;
        }
    }

    bool RunProgressManager::IsStageCleared(int slot) const
    {
        if (slot >= 0 && slot < StageSlotCount)
        {
            return stageCleared[slot];
        }
        return false;
    }

    int RunProgressManager::GetStageConfigIdForSlot(int slot) const
    {
        if (slot < 0 || slot >= StageSlotCount) return 0;
        if (!stageRunInitialized) return std::min(slot, StageSlotCount - 1);
        return stageOrder[slot];
    }

    int RunProgressManager::GetSlotForStageIndex(int stageIdx) const
    {
        if (!stageRunInitialized) return std::clamp(stageIdx, 0, StageSlotCount - 1);
        for (int i = 0; i < StageSlotCount; ++i)
        {
            if (stageOrder[i] == stageIdx) return i;
        }
        return 0;
    }
}