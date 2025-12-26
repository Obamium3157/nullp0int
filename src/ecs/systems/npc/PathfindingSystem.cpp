//
// Created by obamium3157 on 27.12.2025.
//

#include "PathfindingSystem.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_set>
#include <vector>

#include <SFML/System/Vector2.hpp>

#include "../../Components.h"
#include "../../../constants.h"
#include "../../../math/mathUtils.h"

namespace
{
  struct Grid
  {
    int w = 0;
    int h = 0;
    float tileSize = 64.f;
    const ecs::TilemapComponent* map = nullptr;

    [[nodiscard]] bool inBounds(const int x, const int y) const
    {
      return x >= 0 && y >= 0 && x < w && y < h;
    }

    // TilemapComponent::isWall() returns false for out-of-bounds, which is unsafe for path/LOS.
    [[nodiscard]] bool blocked(const int x, const int y) const
    {
      if (!inBounds(x, y)) return true;
      return map ? map->isWall(x, y) : true;
    }

    [[nodiscard]] int idx(const int x, const int y) const
    {
      return y * w + x;
    }

    [[nodiscard]] sf::Vector2f tileCenterWorld(const int x, const int y) const
    {
      return sf::Vector2f{
        (static_cast<float>(x) + 0.5f) * tileSize,
        (static_cast<float>(y) + 0.5f) * tileSize
      };
    }
  };

  [[nodiscard]] ecs::Entity findPlayer(const ecs::Registry& registry)
  {
    for (const auto& e : registry.entities())
    {
      if (registry.hasComponent<ecs::PlayerTag>(e)) return e;
    }
    return ecs::INVALID_ENTITY;
  }

  [[nodiscard]] float length(const sf::Vector2f v)
  {
    return std::sqrt(v.x * v.x + v.y * v.y);
  }

  [[nodiscard]] sf::Vector2f normalizedOrZero(const sf::Vector2f v)
  {
    const float len = length(v);
    if (!std::isfinite(len) || len <= static_cast<float>(SMALL_EPSILON)) return {0.f, 0.f};
    return { v.x / len, v.y / len };
  }

  [[nodiscard]] bool hasLineOfSight(const Grid& g, const sf::Vector2i from, const sf::Vector2i to)
  {
    int x0 = from.x;
    int y0 = from.y;
    const int x1 = to.x;
    const int y1 = to.y;

    if (!g.inBounds(x0, y0) || !g.inBounds(x1, y1)) return false;

    const int dx = std::abs(x1 - x0);
    const int sx = (x0 < x1) ? 1 : -1;

    const int dy = -std::abs(y1 - y0);
    const int sy = (y0 < y1) ? 1 : -1;

    int err = dx + dy;

    while (x0 != x1 || y0 != y1)
    {
      const int e2 = 2 * err;

      if (e2 >= dy)
      {
        err += dy;
        x0 += sx;
      }

      if (e2 <= dx)
      {
        err += dx;
        y0 += sy;
      }

      if (g.blocked(x0, y0)) return false;
    }

    return true;
  }

  [[nodiscard]] bool passesFovCone(
    ecs::Registry& registry,
    const ecs::Entity enemy,
    const ecs::EnemyComponent& enemyComp,
    const sf::Vector2f toPlayerDir
  )
  {
    if (enemyComp.fovDegrees >= 360.f) return true;

    const auto* rot = registry.getComponent<ecs::RotationComponent>(enemy);
    if (!rot)
    {
      return true;
    }

    const float forwardRad = radiansFromDegrees(rot->angle);
    const sf::Vector2f forward{ std::cos(forwardRad), std::sin(forwardRad) };

    const float dot = std::clamp(forward.x * toPlayerDir.x + forward.y * toPlayerDir.y, -1.f, 1.f);
    const float angleRad = std::acos(dot);
    const float angleDeg = angleRad * 180.f / 3.14159265358979323846f;

    return angleDeg <= (enemyComp.fovDegrees * 0.5f);
  }
}

void ecs::PathfindingSystem::update(Registry& registry, const Entity tilemapEntity)
{
  const Entity player = findPlayer(registry);
  if (player == INVALID_ENTITY) return;

  const auto* playerPos = registry.getComponent<PositionComponent>(player);
  if (!playerPos) return;

  const auto* tilemap = registry.getComponent<TilemapComponent>(tilemapEntity);
  if (!tilemap) return;

  Grid g;
  g.w = static_cast<int>(tilemap->width);
  g.h = static_cast<int>(tilemap->height);
  g.tileSize = tilemap->tileSize;
  g.map = tilemap;

  if (g.w <= 0 || g.h <= 0) return;

  const sf::Vector2i playerTile = tilemap->worldToTile(playerPos->position);
  if (!g.inBounds(playerTile.x, playerTile.y)) return;

  const sf::Vector2i kDirs8[8] = {
    { 1,  0}, {-1,  0}, { 0,  1}, { 0, -1},
    { 1,  1}, { 1, -1}, {-1,  1}, {-1, -1}
  };

  struct NeedPath
  {
    Entity e = INVALID_ENTITY;
    PositionComponent* pos = nullptr;
    VelocityComponent* vel = nullptr;
    const SpeedComponent* speed = nullptr;
  };

  std::vector<NeedPath> needPathEnemies;
  needPathEnemies.reserve(64);

  bool needDistanceField = false;

  for (const auto& e : registry.entities())
  {
    if (!registry.hasComponent<EnemyTag>(e)) continue;

    auto* vel = registry.getComponent<VelocityComponent>(e);
    auto* pos = registry.getComponent<PositionComponent>(e);
    const auto* speed = registry.getComponent<SpeedComponent>(e);
    auto* enemyComp = registry.getComponent<EnemyComponent>(e);

    if (!vel || !pos || !speed || !enemyComp) continue;

    const sf::Vector2f toPlayer = playerPos->position - pos->position;
    const float distWorld = std::hypot(toPlayer.x, toPlayer.y);

    const float visionRangeWorld = enemyComp->visionRangeTiles * g.tileSize;

    const bool withinVisionRange = std::isfinite(distWorld) && distWorld <= visionRangeWorld;

    const sf::Vector2i enemyTile = tilemap->worldToTile(pos->position);
    const bool losNow = g.inBounds(enemyTile.x, enemyTile.y) && hasLineOfSight(g, enemyTile, playerTile);

    bool seesPlayerNow = false;
    if (withinVisionRange && losNow)
    {
      const sf::Vector2f toPlayerDir = normalizedOrZero(toPlayer);
      seesPlayerNow = passesFovCone(registry, e, *enemyComp, toPlayerDir);
    }

    if (!enemyComp->hasSeenPlayer && seesPlayerNow)
    {
      enemyComp->hasSeenPlayer = true;
    }

    if (!enemyComp->hasSeenPlayer)
    {
      vel->velocity = {0.f, 0.f};
      continue;
    }

    if (losNow)
    {
      const sf::Vector2f dir = normalizedOrZero(toPlayer);
      if (dir.x == 0.f && dir.y == 0.f)
      {
        vel->velocity = {0.f, 0.f};
        continue;
      }

      const float v = speed->speed * vel->velocityMultiplier;
      vel->velocity = { dir.x * v, dir.y * v };
      continue;
    }

    needDistanceField = true;
    needPathEnemies.push_back(NeedPath{ e, pos, vel, speed });
  }

  if (!needDistanceField) return;

  std::vector dist(static_cast<std::size_t>(g.w) * static_cast<std::size_t>(g.h), -1);

  if (!g.blocked(playerTile.x, playerTile.y))
  {
    std::queue<sf::Vector2i> q;
    dist[g.idx(playerTile.x, playerTile.y)] = 0;
    q.push(playerTile);

    while (!q.empty())
    {
      const sf::Vector2i cur = q.front();
      q.pop();

      const int curD = dist[g.idx(cur.x, cur.y)];

      for (const auto d : kDirs8)
      {
        const int nx = cur.x + d.x;
        const int ny = cur.y + d.y;

        if (!g.inBounds(nx, ny)) continue;
        if (g.blocked(nx, ny)) continue;

        if (d.x != 0 && d.y != 0)
        {
          if (g.blocked(cur.x + d.x, cur.y) || g.blocked(cur.x, cur.y + d.y)) continue;
        }

        const int nIndex = g.idx(nx, ny);
        if (dist[nIndex] != -1) continue;

        dist[nIndex] = curD + 1;
        q.emplace(nx, ny);
      }
    }
  }

  std::unordered_set<int> occupied;
  occupied.reserve(registry.entities().size());

  for (const auto& e : registry.entities())
  {
    if (!registry.hasComponent<EnemyTag>(e)) continue;

    const auto* pos = registry.getComponent<PositionComponent>(e);
    const auto* enemyComp = registry.getComponent<EnemyComponent>(e);
    if (!pos || !enemyComp) continue;

    if (!enemyComp->hasSeenPlayer) continue;

    const sf::Vector2i t = tilemap->worldToTile(pos->position);
    if (!g.inBounds(t.x, t.y)) continue;

    occupied.insert(g.idx(t.x, t.y));
  }

  for (const auto& item : needPathEnemies)
  {
    const sf::Vector2i curTile = tilemap->worldToTile(item.pos->position);
    if (!g.inBounds(curTile.x, curTile.y))
    {
      item.vel->velocity = {0.f, 0.f};
      continue;
    }

    const int curIndex = g.idx(curTile.x, curTile.y);
    const int curDist = dist[curIndex];

    if (curDist <= 0)
    {
      item.vel->velocity = {0.f, 0.f};
      continue;
    }

    sf::Vector2i nextTile = curTile;
    int bestDist = curDist;

    for (const auto d : kDirs8)
    {
      const int nx = curTile.x + d.x;
      const int ny = curTile.y + d.y;

      if (!g.inBounds(nx, ny)) continue;
      if (g.blocked(nx, ny)) continue;

      if (d.x != 0 && d.y != 0)
      {
        if (g.blocked(curTile.x + d.x, curTile.y) || g.blocked(curTile.x, curTile.y + d.y)) continue;
      }

      const int nIndex = g.idx(nx, ny);
      const int nd = dist[nIndex];

      if (nd < 0) continue;
      if (nd >= bestDist) continue;

      if (!(nx == playerTile.x && ny == playerTile.y))
      {
        if (occupied.contains(nIndex) && nIndex != curIndex) continue;
      }

      bestDist = nd;
      nextTile = {nx, ny};
    }

    if (nextTile == curTile)
    {
      item.vel->velocity = {0.f, 0.f};
      continue;
    }

    const sf::Vector2f target = g.tileCenterWorld(nextTile.x, nextTile.y);
    const sf::Vector2f dir = normalizedOrZero(target - item.pos->position);

    if (dir.x == 0.f && dir.y == 0.f)
    {
      item.vel->velocity = {0.f, 0.f};
      continue;
    }

    const float v = item.speed->speed * item.vel->velocityMultiplier;
    item.vel->velocity = { dir.x * v, dir.y * v };
  }
}
