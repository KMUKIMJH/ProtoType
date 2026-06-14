#pragma once
#include "../Engine/GameObject.h"
#include "GameObjectTypes.h"
#include <string>

namespace CS230
{
    class HPStation : public GameObject
    {
    public:
        HPStation(Math::vec2 pos);
        void Update(double dt) override;
        void Draw(Math::TransformationMatrix camM) override;

        GameObjectTypes Type() override
        {
            return GameObjectTypes::HPStation;
        }

        std::string TypeName() override
        {
            return "HPStation";
        }

        const Math::rect& GetRect() const
        {
            return rect;
        }

    private:
        Math::rect rect;
    };
}