#include "MapLoader.h"
#include "StageManager.h"
#include "../Game/Platform.h"
#include "../Game/TwoWayPlatform.h"
#include "../Game/Zipline.h"
#include "../Game/BackDoor.h"
#include "../Game/HPStation.h"
#include "../Game/Generator.h"
#include "../Engine/logger.h"
#include <fstream>
#include <algorithm>
#include <cctype>
#include <cmath>

namespace CS230
{
    std::string MapLoader::GetSVGAttribute(const std::string& tagStr, const std::string& attrName)
    {
        std::string target = attrName + "=\"";
        size_t startPos = tagStr.find(target);

        if (startPos == std::string::npos)
        {
            target = attrName + "='";
            startPos = tagStr.find(target);
            if (startPos == std::string::npos)
            {
                return "";
            }
        }

        startPos += target.length();
        size_t endPos = tagStr.find(target.back(), startPos);

        if (endPos == std::string::npos)
        {
            return "";
        }

        return tagStr.substr(startPos, endPos - startPos);
    }

    unsigned int MapLoader::ParseSVGColor(const std::string& hexStr, unsigned int defaultColor)
    {
        if (hexStr.empty() || hexStr[0] != '#')
        {
            return defaultColor;
        }
        std::string hex = hexStr.substr(1);
        if (hex.length() >= 6)
        {
            try
            {
                unsigned int r = std::stoul(hex.substr(0, 2), nullptr, 16);
                unsigned int g = std::stoul(hex.substr(2, 2), nullptr, 16);
                unsigned int b = std::stoul(hex.substr(4, 2), nullptr, 16);
                return (r << 24) | (g << 16) | (b << 8) | 0xFF;
            }
            catch (...)
            {
                return defaultColor;
            }
        }
        return defaultColor;
    }

    void MapLoader::LoadSVG(const std::string& filepath, StageManager& stage)
    {
        std::ifstream file(filepath);

        if (!file.is_open())
        {
            Engine::GetLogger().LogError("Cannot open SVG: " + filepath);
            return;
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        auto toLowerStr = [](std::string s)
            {
                std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                return s;
            };

        std::vector<std::string> layerStack;
        size_t pos = 0;

        while (pos < content.length())
        {
            size_t tagStart = content.find("<", pos);
            if (tagStart == std::string::npos)
            {
                break;
            }

            size_t tagEnd = content.find(">", tagStart);
            if (tagEnd == std::string::npos)
            {
                break;
            }

            std::string tag = content.substr(tagStart, tagEnd - tagStart + 1);

            if (tag.find("<g") == 0 || tag.find("<g ") == 0 || tag.find("<g\n") == 0 || tag.find("<g\r") == 0)
            {
                std::string layerId = GetSVGAttribute(tag, "id");
                std::string layerLabel = GetSVGAttribute(tag, "inkscape:label");
                std::string dataName = GetSVGAttribute(tag, "data-name");

                std::string layerName = layerId;
                if (!dataName.empty())
                {
                    layerName = dataName;
                }
                if (!layerLabel.empty())
                {
                    layerName = layerLabel;
                }

                layerStack.push_back(toLowerStr(layerName));

                if (tag.length() >= 2 && tag.substr(tag.length() - 2) == "/>")
                {
                    layerStack.pop_back();
                }
            }
            else if (tag.find("</g>") != std::string::npos)
            {
                if (!layerStack.empty())
                {
                    layerStack.pop_back();
                }
            }
            else if (tag.find("<rect") == 0 || tag.find("<rect ") == 0 || tag.find("<rect\n") == 0)
            {
                std::string rectId = GetSVGAttribute(tag, "id");
                std::string dataName = GetSVGAttribute(tag, "data-name");
                std::string className = GetSVGAttribute(tag, "class");
                if (!dataName.empty())
                {
                    rectId = dataName;
                }

                std::string typeStr = "";
                for (const auto& l : layerStack)
                {
                    typeStr += l + "_";
                }
                typeStr += rectId + "_" + className;
                typeStr = toLowerStr(typeStr);

                if (typeStr.find("background") != std::string::npos)
                {
                    pos = tagEnd + 1;
                    continue;
                }

                std::string xStr = GetSVGAttribute(tag, "x");
                std::string yStr = GetSVGAttribute(tag, "y");
                std::string wStr = GetSVGAttribute(tag, "width");
                std::string hStr = GetSVGAttribute(tag, "height");

                if (wStr.empty() || hStr.empty())
                {
                    pos = tagEnd + 1;
                    continue;
                }

                double svgX = xStr.empty() ? 0.0 : std::stod(xStr);
                double svgY = yStr.empty() ? 0.0 : std::stod(yStr);
                double width = std::stod(wStr);
                double height = std::stod(hStr);

                int engineX = static_cast<int>(std::round(svgX));
                int engineY = static_cast<int>(std::round(stage.worldHeightCur - svgY - height));

                if (typeStr.find("zipline") != std::string::npos)
                {
                    Math::irect zipRect{ { static_cast<int>(engineX), static_cast<int>(engineY) }, { static_cast<int>(engineX + width), static_cast<int>(engineY + height) } };
                    stage.staticObjects.push_back(new Zipline(zipRect));
                }
                else if (typeStr.find("twoway") != std::string::npos)
                {
                    Math::irect platRect{ { static_cast<int>(engineX), static_cast<int>(engineY) }, { static_cast<int>(engineX + width), static_cast<int>(engineY + height) } };
                    stage.staticObjects.push_back(new TwoWayPlatform(platRect));
                }
                else if (typeStr.find("platform") != std::string::npos || typeStr.find("floor") != std::string::npos || typeStr.find("wall") != std::string::npos)
                {
                    Math::irect platRect{ { engineX, engineY }, { static_cast<int>(engineX + width), static_cast<int>(engineY + height) } };
                    stage.AddPlatform(platRect);
                }
                else if (typeStr.find("spawn") != std::string::npos || typeStr.find("player") != std::string::npos)
                {
                    stage.hasPlayerSpawnOverride = true;
                    stage.playerSpawnOverride = { static_cast<double>(engineX), static_cast<double>(engineY) };
                }
                else if (typeStr.find("meleestoryenemy") != std::string::npos)
                {
                    stage.AddEnemy({ static_cast<double>(engineX), static_cast<double>(engineY) }, EnemyType::Melee, true);
                }
                else if (typeStr.find("meleeenemy") != std::string::npos || typeStr.find("melee") != std::string::npos)
                {
                    bool isStory = (typeStr.find("story") != std::string::npos);
                    stage.AddEnemy({ static_cast<double>(engineX), static_cast<double>(engineY) }, EnemyType::Melee, isStory);
                }
                else if (typeStr.find("rangedstoryenemy") != std::string::npos)
                {
                    stage.AddEnemy({ static_cast<double>(engineX), static_cast<double>(engineY) }, EnemyType::Ranged, true);
                }
                else if (typeStr.find("rangedenemy") != std::string::npos || typeStr.find("ranged") != std::string::npos)
                {
                    bool isStory = (typeStr.find("story") != std::string::npos);
                    stage.AddEnemy({ static_cast<double>(engineX), static_cast<double>(engineY) }, EnemyType::Ranged, isStory);
                }
                else if (typeStr.find("shieldstoryenemy") != std::string::npos)
                {
                    stage.AddEnemy({ static_cast<double>(engineX), static_cast<double>(engineY) }, EnemyType::Shield, true);
                }
                else if (typeStr.find("shieldenemy") != std::string::npos || typeStr.find("shield") != std::string::npos)
                {
                    bool isStory = (typeStr.find("story") != std::string::npos);
                    stage.AddEnemy({ static_cast<double>(engineX), static_cast<double>(engineY) }, EnemyType::Shield, isStory);
                }
                else if (typeStr.find("portal") != std::string::npos)
                {
                    stage.AddPortal({ engineX, engineY });
                }
                else if (typeStr.find("hpstation") != std::string::npos || typeStr.find("hp") != std::string::npos)
                {
                    auto* station = new CS230::HPStation({ static_cast<double>(engineX), static_cast<double>(engineY) });
                    stage.staticObjects.push_back(station);
                }
                else if (typeStr.find("generator") != std::string::npos || typeStr.find("shop") != std::string::npos)
                {
                    if (!stage.generator)
                    {
                        stage.generator = new CS230::Generator({ static_cast<double>(engineX), static_cast<double>(engineY) });
                        stage.generator->RollUpgrades(&stage.player);
                    }
                }
                else if (typeStr.find("backdoor") != std::string::npos)
                {
                    bool isDuplicate = false;
                    for (auto* existingBd : stage.backdoors)
                    {
                        if (std::abs(existingBd->GetRect().Left() - engineX) < 10.0 && std::abs(existingBd->GetRect().Bottom() - engineY) < 10.0)
                        {
                            isDuplicate = true;
                            break;
                        }
                    }

                    if (!isDuplicate)
                    {
                        int bdIndex = static_cast<int>(stage.backdoors.size() + 1);
                        std::string bdName = "Area " + std::to_string(bdIndex);

                        Math::rect r{ { static_cast<double>(engineX), static_cast<double>(engineY) }, { static_cast<double>(engineX + width), static_cast<double>(engineY + height) } };
                        auto* bd = new CS230::Backdoor(r, bdIndex, bdName);
                        stage.backdoors.push_back(bd);
                        stage.staticObjects.push_back(bd);
                    }
                }
            }
            else if (tag.find("<text") == 0 || tag.find("<text ") == 0 || tag.find("<text\n") == 0)
            {
                std::string xStr = GetSVGAttribute(tag, "x");
                std::string yStr = GetSVGAttribute(tag, "y");
                std::string fillStr = GetSVGAttribute(tag, "fill");
                std::string fsStr = GetSVGAttribute(tag, "font-size");

                size_t textEndTag = content.find("</text>", tagEnd);
                if (textEndTag != std::string::npos)
                {
                    std::string textContent = content.substr(tagEnd + 1, textEndTag - tagEnd - 1);

                    size_t ampPos = textContent.find("&amp;");
                    while (ampPos != std::string::npos)
                    {
                        textContent.replace(ampPos, 5, "&");
                        ampPos = textContent.find("&amp;", ampPos + 1);
                    }

                    if (!xStr.empty() && !yStr.empty())
                    {
                        double svgX = std::stod(xStr);
                        double svgY = std::stod(yStr);
                        double engineX = svgX;
                        double engineY = stage.worldHeightCur - svgY;

                        unsigned int color = ParseSVGColor(fillStr, 0xFFFFFFFF);
                        double scale = fsStr.empty() ? 0.6 : (std::stod(fsStr) / 60.0);

                        stage.AddText(textContent, { engineX, engineY }, color, scale);
                    }
                    pos = textEndTag + 7;
                    continue;
                }
            }

            pos = tagEnd + 1;
        }

        if (!stage.backdoors.empty())
        {
            std::sort(stage.backdoors.begin(), stage.backdoors.end(), [](CS230::Backdoor* a, CS230::Backdoor* b) { return a->GetIndex() < b->GetIndex(); });
        }

        Engine::GetLogger().LogEvent("Successfully parsed SVG map: " + filepath);
    }
}