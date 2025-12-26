//
// Created by obamium3157 on 13.11.2025.
//

#pragma once

#ifndef NULLP0INT_COMPONENTS_H
#define NULLP0INT_COMPONENTS_H

#include <cmath>
#include <unordered_map>
#include <SFML/Graphics/RenderWindow.hpp>

#include "systems/render/Animation.h"

namespace ecs
{
  struct PositionComponent
  {
    sf::Vector2f position{};
  };

  struct VelocityComponent
  {
    sf::Vector2f velocity{};
    float velocityMultiplier = 1.f;
  };
  struct RotationVelocityComponent
  {
    float rotationVelocity = 0.f;
  };

  struct SpeedComponent
  {
    float speed = 0.f;
  };
  struct RotationSpeedComponent
  {
    float rotationSpeed = 0.f;
  };

  struct RotationComponent
  {
    float angle = 0.f;
  };

  struct RadiusComponent
  {
    float radius = 0.f;
  };

  struct PlayerTag{};
  struct PlayerInput
  {
    float moveSpeed = 150.f;
  };

  enum class EnemyClass
  {
    MELEE,
    RANGE,
    SUPPORT,
  };

  struct EnemyComponent
  {
    EnemyClass cls = EnemyClass::MELEE;
    std::string textureId;
    float spriteScale = 1.f;
    float heightShift = 0.27f;

    bool hasSeenPlayer = false;
    float visionRangeTiles = 40.f;
    float fovDegrees = 360.f;
  };
  struct EnemyTag{};


  struct TilemapTag{};


  struct TileAppearance
  {
    std::string singleTextureId{};
    std::vector<std::string> frames;
    float frameTime = 0.1f;
    float lastUpdateTime = 0.f;

    [[nodiscard]] bool isAnimated() const { return !frames.empty(); }
    [[nodiscard]] const std::string& currentTextureId(const float currentTime) const
    {
      if (!isAnimated()) return singleTextureId;
      if (frameTime <= 0.f) return frames.front();

      const float localTime = currentTime - lastUpdateTime;
      const size_t idx = static_cast<size_t>(std::floor(localTime / frameTime)) % frames.size();
      return frames[idx];
    }
  };

  struct TilemapComponent
  {
    unsigned width = 0;
    unsigned height = 0;
    float tileSize = 64.f;
    std::vector<std::string> tiles;
    std::string floorTextureId;

    std::unordered_map<char, TileAppearance> tileAppearanceMap;

    [[nodiscard]] bool isWall(const int tx, const int ty) const
    {
      if (tx < 0 || ty < 0 || tx >= static_cast<int>(width) || ty >= static_cast<int>(height))
        return false;

      return tiles[ty][tx] != ' ' && tiles[ty][tx] != '*';
    }

    [[nodiscard]] sf::Vector2f getSpawnPosition() const
    {
      for (unsigned ty = 0; ty < height; ++ty)
      {
        for (unsigned tx = 0; tx < width; ++tx)
        {
          if (tiles[ty][tx] == '*')
          {
            return sf::Vector2f{
              static_cast<float>(tx) * tileSize + tileSize / 2.f,
              static_cast<float>(ty) * tileSize + tileSize / 2.f
            };
          }
        }
      }

      return sf::Vector2f{
        static_cast<float>(width) * tileSize / 2.f,
        static_cast<float>(height) * tileSize / 2.f
      };
    }

    [[nodiscard]] sf::Vector2i worldToTile(const sf::Vector2f worldPos) const
    {
      return {static_cast<int>(worldPos.x / tileSize), static_cast<int>(worldPos.y / tileSize)};
    }
  };

  struct RayHit
  {
    sf::Vector2f hitPointWorld;
    float distance = 0.f;
    int tileX = -1;
    int tileY = -1;
    bool vertical = false;
    float rayAngle = 0.f;
  };

  struct RayCastResultComponent
  {
    std::vector<RayHit> hits;
  };


  struct SpriteComponent
  {
    std::string textureId;
    std::vector<sf::IntRect> frames;
    std::vector<std::string> textureFrames;
    float frameTime = 0.1f;
    bool playing = true;
    bool loop = true;
    std::size_t currentFrame = 0;
    float frameAccumulator = 0.f;
  };
}

#endif //NULLP0INT_COMPONENTS_H
