//
// Created by obamium3157 on 14.11.2025.
//

#include "CollisionSystem.h"
#include <cmath>

#include "../../Components.h"

bool ecs::CollisionSystem::checkWallCollision(Registry &registry, const sf::Vector2f position, const float radius, Entity tilemap)
{
  const auto* map = registry.getComponent<TilemapComponent>(tilemap);
  if (!map) return false;

  const float cx = position.x;
  const float cy = position.y;
  const float ts = map->tileSize;

  const int minTileX = static_cast<int>(std::floor((cx - radius) / ts));
  const int maxTileX = static_cast<int>(std::ceil((cx + radius) / ts));
  const int minTileY = static_cast<int>(std::floor((cy - radius) / ts));
  const int maxTileY = static_cast<int>(std::ceil((cy + radius) / ts));

  if (minTileX < 0 || minTileY < 0 || maxTileX >= static_cast<int>(map->width) || maxTileY >= static_cast<int>(map->height))
  {
    return true;
  }

  for (int ty = minTileY; ty <= maxTileY; ++ty)
  {
    for (int tx = minTileX; tx <= maxTileX; ++tx)
    {
      if (ty >= map->tiles.size() || tx >= map->tiles[ty].length()) return true;

      if (!map->isWall(tx, ty)) continue;


      const auto tileLeft = static_cast<float>(tx) * ts;
      const auto tileRight = tileLeft + ts;
      const auto tileTop = static_cast<float>(ty) * ts;
      const auto tileBottom = tileTop + ts;

      const float nearestX = std::clamp(cx, tileLeft, tileRight);
      const float nearestY = std::clamp(cy, tileTop, tileBottom);

      const float dx = cx - nearestX;
      const float dy = cy - nearestY;

      if (dx * dx + dy * dy < radius * radius)
      {
        return true;
      }
    }
  }

  return false;
}
