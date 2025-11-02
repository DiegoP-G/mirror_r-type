#pragma once
#include <filesystem>
#include <string>

const char backgroundSpritePath[] = "assets/sprites/background.jpg";

const char playerSpritePath[] = "assets/sprites/playerSpritesheet.png";
const char fontPath[] = "assets/fonts/upheavtt.ttf";
const char bulletSpritePath[] = "assets/sprites/bullet_spritesheet.png";
const char basicEnemySpritePath[] = "assets/sprites/basic_enemy_spritesheet.png";
const char bonusBubblesSpritePath[] = "assets/sprites/bonus_bubbles.png";
const char bossSpritePath[] = "assets/sprites/spaceship_boss.png";
const char explosionSpritePath[] = "assets/sprites/explosion.png";
const char rotatingEnemySpritePath[] = "assets/sprites/rotating_enemy.png";
const char purpleEnemySpritePath[] = "assets/sprites/purple_enemy.png";
const char shieldSpritePath[] = "assets/sprites/shield.png";

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
