#pragma once
#include <filesystem>
#include <string>

const char backgroundSpritePath[] = "assets/sprites/background_starry.jpg";
const char playerSpritePath[] = "assets/sprites/playerSpritesheet.png";

class PathFormater {
public:
  static std::string formatAssetPath(const std::string &relativePath) {
    std::string basePath = std::filesystem::current_path().string();
    return basePath + "/" + relativePath;
  }
};
