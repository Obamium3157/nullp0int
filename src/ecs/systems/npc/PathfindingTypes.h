//
// Created by obamium3157 on 09.01.2026.
//

#ifndef NULLP0INT_PATHFINDINGTYPES_H
#define NULLP0INT_PATHFINDINGTYPES_H

#include <algorithm>
#include <cmath>

#include <SFML/System/Vector2.hpp>

#include "../../Components.h"

namespace ecs::npc
{
  inline constexpr float NORMALiZE_EPS = 1e-6f;

  struct Grid
  {
    int w = 0;
    int h = 0;
    float tileSize = 64.f;
    const TilemapComponent* map = nullptr;

    [[nodiscard]] bool inBounds(const int x, const int y) const
    {
      return x >= 0 && y >= 0 && x < w && y < h;
    }

    [[nodiscard]] int idx(const int x, const int y) const
    {
      return x + y * w;
    }

    [[nodiscard]] bool blocked(const int x, const int y) const
    {
      return map ? map->isWall(x, y) : true;
    }

    [[nodiscard]] sf::Vector2f tileCenterWorld(const int tx, const int ty) const
    {
      return sf::Vector2f{
        (static_cast<float>(tx) + 0.5f) * tileSize,
        (static_cast<float>(ty) + 0.5f) * tileSize
      };
    }
  };

  [[nodiscard]] inline sf::Vector2f normalizedOrZero(const sf::Vector2f v, const float eps = NORMALiZE_EPS)
  {
    const float len = std::hypot(v.x, v.y);
    if (len <= eps) return {0.f, 0.f};
    return v / len;
  }

  [[nodiscard]] inline float clamp01(const float x)
  {
    return std::clamp(x, 0.f, 1.f);
  }

  struct OrbitTuning
  {
    float rangeWeight = 1.f;
    float tangentialWeight = 0.7f;
    float wrongSidePenalty = 0.4f;
    float driftWeight = 0.15f;
    float toleranceExpand = 2.0f;
  };

  inline constexpr OrbitTuning DEFAULT_ORBIT_TUNING{};
}

#endif //NULLP0INT_PATHFINDINGTYPES_H