#pragma once
#include <array>
#include <vector>

namespace CS230
{
    class RunProgressManager
    {
    public:
        RunProgressManager();

        static constexpr int StageSlotCount = 10;
        static constexpr int TutorialConfigId = 7;

        void ResetRun();
        void GenerateStageRun();
        void RerollPostMidBoss();

        int GetStageConfigIdForSlot(int slot) const;
        int GetSlotForStageIndex(int stageIdx) const;

        void MarkStageCleared(int slot);
        bool IsStageCleared(int slot) const;

        std::vector<std::vector<bool>> enemyAliveStatus;

    private:
        int stageOrder[StageSlotCount];
        std::array<bool, StageSlotCount> stageCleared;
        bool stageRunInitialized{ false };
    };
}