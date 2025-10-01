#pragma once
#include <filesystem>
#include <string>

const char playerSpritePath[] = "assets/sprites/playerSpritesheet.png";
const char fontPath[] = "assets/fonts/upheavtt.ttf";
const char bulletSpritePath[] = "assets/sprites/bullet_spritesheet.png";

class PathFormater
{
  public:
    static std::string formatAssetPath(const std::string &relativePath)
    {
        std::filesystem::path currentPath = std::filesystem::current_path();
        while (!std::filesystem::exists(currentPath / ".git") && currentPath.has_parent_path())
        {
            currentPath = currentPath.parent_path();
        }

        if (std::filesystem::exists(currentPath / ".git"))
        {
            return (currentPath / relativePath).string();
        }

        return relativePath;
    }
};