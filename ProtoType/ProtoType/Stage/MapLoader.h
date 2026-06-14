#pragma once
#include <string>

class StageManager;

namespace CS230
{
    class MapLoader
    {
    public:
        static void LoadSVG(const std::string& filepath, StageManager& stage);
        static std::string GetSVGAttribute(const std::string& tagStr, const std::string& attrName);

    private:
        static unsigned int ParseSVGColor(const std::string& hexStr, unsigned int defaultColor);
    };
}