//
// Created by obamium3157 on 09.01.2026.
//

#include "PathfindingPerception.h"

#include "PathfindingTypes.h"
#include "../../Components.h"
#include "../../Entity.h"
#include "../../Registry.h"
#include "../../../math/mathUtils.h"

namespace ecs::npc
{
  Entity findPlayer(const Registry& registry)
  {
    for (const auto& e : registry.entities())
    {
      if (registry.hasComponent<PlayerTag>(e)) return e;
    }
    return INVALID_ENTITY;
  }

  static float clampDot(const float d)
  {
    return std::clamp(d, -1.f, 1.f);
  }

  bool passesFovCone(
    Registry& registry,
    const Entity enemyEnt,
    const EnemyComponent& enemyComp,
    const sf::Vector2f toPlayerDir
  )
  {
    const auto* rot = registry.getComponent<RotationComponent>(enemyEnt);
    if (!rot) return true;

    const float fov = enemyComp.fovDegrees;
    if (fov >= 359.9f) return true;

    const float enemyAngleRad = radiansFromDegrees(rot->angle);
    const sf::Vector2f forward{ std::cos(enemyAngleRad), std::sin(enemyAngleRad) };

    const float dot = forward.x * toPlayerDir.x + forward.y * toPlayerDir.y;
    const float angle = std::acos(clampDot(dot));

    const float halfFovRad = radiansFromDegrees(fov * 0.5f);
    return angle <= halfFovRad;
  }

  bool hasLineOfSightWorld(const TilemapComponent& map, const sf::Vector2f fromWorld, const sf::Vector2f toWorld)
  {
    const auto tileSize = static_cast<double>(map.tileSize);
    if (tileSize <= 0.0) return false;

    const sf::Vector2f dWorld = toWorld - fromWorld;
    const double distWorld = std::hypot(static_cast<double>(dWorld.x), static_cast<double>(dWorld.y));
    if (!std::isfinite(distWorld) || distWorld <= BIG_EPSILON) return true;

    const double dirX = static_cast<double>(dWorld.x) / distWorld;
    const double dirY = static_cast<double>(dWorld.y) / distWorld;

    constexpr double kNudgeTiles = BIG_EPSILON;
    const double rayX = static_cast<double>(fromWorld.x) / tileSize + dirX * kNudgeTiles;
    const double rayY = static_cast<double>(fromWorld.y) / tileSize + dirY * kNudgeTiles;

    const int targetTileX = static_cast<int>(std::floor(static_cast<double>(toWorld.x) / tileSize));
    const int targetTileY = static_cast<int>(std::floor(static_cast<double>(toWorld.y) / tileSize));

    int mapX = static_cast<int>(std::floor(rayX));
    int mapY = static_cast<int>(std::floor(rayY));

    if (mapX == targetTileX && mapY == targetTileY) return true;

    const int stepX = (dirX >= 0.0) ? 1 : -1;
    const int stepY = (dirY >= 0.0) ? 1 : -1;

    const double deltaDistX = (std::abs(dirX) < BIGGER_EPSILON) ? std::numeric_limits<double>::infinity() : std::abs(1.0 / dirX);
    const double deltaDistY = (std::abs(dirY) < BIGGER_EPSILON) ? std::numeric_limits<double>::infinity() : std::abs(1.0 / dirY);

    const double nextGridX = (stepX > 0) ? (std::floor(rayX) + 1.0) : std::floor(rayX);
    const double nextGridY = (stepY > 0) ? (std::floor(rayY) + 1.0) : std::floor(rayY);

    double sideDistX = (std::abs(dirX) < BIGGER_EPSILON) ? std::numeric_limits<double>::infinity() : std::abs(nextGridX - rayX) * deltaDistX;
    double sideDistY = (std::abs(dirY) < BIGGER_EPSILON) ? std::numeric_limits<double>::infinity() : std::abs(nextGridY - rayY) * deltaDistY;

    const double maxDistTiles = distWorld / tileSize;
    double traveledTiles = 0.0;

    while (traveledTiles <= maxDistTiles + 1e-6)
    {
      if (std::abs(sideDistX - sideDistY) <= BIGGER_EPSILON)
      {
        const int nextX = mapX + stepX;
        const int nextY = mapY + stepY;

        traveledTiles = sideDistX;
        sideDistX += deltaDistX;
        sideDistY += deltaDistY;

        if (nextX < 0 || mapY < 0 || nextX >= static_cast<int>(map.width) || mapY >= static_cast<int>(map.height)) return false;
        if (mapX < 0 || nextY < 0 || mapX >= static_cast<int>(map.width) || nextY >= static_cast<int>(map.height)) return false;

        if (map.isWall(nextX, mapY)) return false;
        if (map.isWall(mapX, nextY)) return false;

        mapX = nextX;
        mapY = nextY;

        if (map.isWall(mapX, mapY)) return false;

        if (mapX == targetTileX && mapY == targetTileY) return true;
        continue;
      }

      if (sideDistX < sideDistY)
      {
        mapX += stepX;
        traveledTiles = sideDistX;
        sideDistX += deltaDistX;
      }
      else
      {
        mapY += stepY;
        traveledTiles = sideDistY;
        sideDistY += deltaDistY;
      }

      if (mapX < 0 || mapY < 0 || mapX >= static_cast<int>(map.width) || mapY >= static_cast<int>(map.height)) return false;
      if (map.isWall(mapX, mapY)) return false;
      if (mapX == targetTileX && mapY == targetTileY) return true;
    }

    return false;
  }

  PerceptionResult computePerception(
    Registry& registry,
    const Entity enemyEnt,
    const PositionComponent& enemyPos,
    const EnemyComponent& enemyComp,
    const TilemapComponent& map,
    const PositionComponent& playerPos
  )
  {
    PerceptionResult r;

    r.toPlayer = playerPos.position - enemyPos.position;
    r.distWorld = std::hypot(r.toPlayer.x, r.toPlayer.y);
    r.toPlayerDir = normalizedOrZero(r.toPlayer);
    r.distTilesEuclid = (map.tileSize > 0.f) ? (r.distWorld / map.tileSize) : 0.f;

    const float visionRangeWorld = enemyComp.visionRangeTiles * map.tileSize;
    r.withinVisionRange = std::isfinite(r.distWorld) && (r.distWorld <= visionRangeWorld);

    r.los = hasLineOfSightWorld(map, enemyPos.position, playerPos.position);

    if (r.withinVisionRange && r.los)
    {
      r.seesPlayerNow = passesFovCone(registry, enemyEnt, enemyComp, r.toPlayerDir);
    }

    return r;
  }
}
