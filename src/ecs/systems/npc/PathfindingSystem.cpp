//
// Created by obamium3157 on 27.12.2025.
//

#include "PathfindingSystem.h"

#include <deque>
#include <unordered_set>

#include "../../Components.h"
#include "../../../constants.h"

namespace
{
  struct TilePos
  {
    int x = 0;
    int y = 0;

    friend bool operator==(const TilePos& a, const TilePos& b)
    {
      return a.x == b.x && a.y == b.y;
    }
  };

  [[nodiscard]] int tileIndex(const int x, const int y, const int w)
  {
    return y * w + x;
  }

  [[nodiscard]] bool isPassable(const ecs::TilemapComponent& map, const int x, const int y)
  {
    if (x < 0 || y < 0) return false;
    if (x >= static_cast<int>(map.width) || y >= static_cast<int>(map.height)) return false;
    if (y >= static_cast<int>(map.tiles.size())) return false;
    if (x >= static_cast<int>(map.tiles[y].size())) return false;
    return !map.isWall(x, y);
  }

  [[nodiscard]] sf::Vector2f tileCenterWorld(const ecs::TilemapComponent& map, const int x, const int y)
  {
    const float ts = map.tileSize;
    return { (static_cast<float>(x) + 0.5f) * ts, (static_cast<float>(y) + 0.5f) * ts };
  }

  [[nodiscard]] float lengthSq(const sf::Vector2f v)
  {
    return v.x * v.x + v.y * v.y;
  }

  [[nodiscard]] bool canMove(const ecs::TilemapComponent& map, const TilePos from, const TilePos to)
  {
    if (!isPassable(map, to.x, to.y)) return false;

    const int dx = to.x - from.x;
    const int dy = to.y - from.y;

    if (dx == 0 || dy == 0) return true;

    return isPassable(map, from.x + dx, from.y) && isPassable(map, from.x, from.y + dy);
  }
}


void ecs::PathfindingSystem::update(Registry& registry, const Entity tilemap)
{
  const auto* map = registry.getComponent<TilemapComponent>(tilemap);
  if (!map || map->width == 0 || map->height == 0) return;

  Entity player = INVALID_ENTITY;
  const auto& ents = registry.entities();
  for (const auto& e : ents)
  {
    if (registry.hasComponent<PlayerTag>(e))
    {
      player = e;
      break;
    }
  }
  if (player == INVALID_ENTITY) return;

  const auto* playerPos = registry.getComponent<PositionComponent>(player);
  if (!playerPos) return;

  const sf::Vector2i playerTileI = map->worldToTile(playerPos->position);
  const TilePos playerTile{ playerTileI.x, playerTileI.y };

  if (!isPassable(*map, playerTile.x, playerTile.y)) return;

  std::unordered_set<int> occupied;
  occupied.reserve(64);
  for (const auto& e : ents)
  {
    if (!registry.hasComponent<EnemyTag>(e)) continue;
    const auto* pos = registry.getComponent<PositionComponent>(e);
    if (!pos) continue;
    const sf::Vector2i t = map->worldToTile(pos->position);
    if (!isPassable(*map, t.x, t.y)) continue;
    occupied.insert(tileIndex(t.x, t.y, static_cast<int>(map->width)));
  }

  const int w = static_cast<int>(map->width);
  const int h = static_cast<int>(map->height);
  const int total = w * h;

  static std::vector<int> dist;
  static int cachedW = 0;
  static int cachedH = 0;
  static Entity cachedTilemap = INVALID_ENTITY;
  static TilePos cachedPlayerTile{ std::numeric_limits<int>::min(), std::numeric_limits<int>::min() };

  const bool mapChanged = (cachedTilemap != tilemap) || (cachedW != w) || (cachedH != h);

  if (const bool playerMovedTile = cachedPlayerTile != playerTile;
    mapChanged
    || playerMovedTile
    || static_cast<int>(dist.size()) != total)
  {
    dist.assign(total, -1);

    std::pmr::deque<TilePos> q;
    dist[tileIndex(playerTile.x, playerTile.y, w)] = 0;
    q.push_back(playerTile);

    constexpr int kDirs[8][2] = {
      { -1,  0 }, {  1,  0 }, { 0, -1 }, { 0,  1 },
      { -1, -1 }, {  1, -1 }, { 1,  1 }, { -1, 1 }
    };

    while (!q.empty())
    {
      const TilePos cur = q.front();
      q.pop_front();

      const int curIdx = tileIndex(cur.x, cur.y, w);
      const int curD = dist[curIdx];

      for (const auto& d : kDirs)
      {
        const TilePos nxt{ cur.x + d[0], cur.y + d[1] };
        if (!canMove(*map, cur, nxt)) continue;

        const int ni = tileIndex(nxt.x, nxt.y, w);
        if (ni < 0 || ni >= total) continue;
        if (dist[ni] != -1) continue;

        dist[ni] = curD + 1;
        q.push_back(nxt);
      }
    }

    cachedW = w;
    cachedH = h;
    cachedTilemap = tilemap;
    cachedPlayerTile = playerTile;
  }

  constexpr int kDirs[8][2] = {
    { -1,  0 }, {  1,  0 }, { 0, -1 }, { 0,  1 },
    { -1, -1 }, {  1, -1 }, { 1,  1 }, { -1, 1 }
  };

  for (const auto& e : ents)
  {
    if (!registry.hasComponent<EnemyTag>(e)) continue;

    auto* vel = registry.getComponent<VelocityComponent>(e);
    const auto* speed = registry.getComponent<SpeedComponent>(e);
    const auto* pos = registry.getComponent<PositionComponent>(e);
    if (!vel || !speed || !pos) continue;

    const sf::Vector2i enemyTileI = map->worldToTile(pos->position);
    const TilePos enemyTile{ enemyTileI.x, enemyTileI.y };
    if (!isPassable(*map, enemyTile.x, enemyTile.y))
    {
      vel->velocity = { 0.f, 0.f };
      continue;
    }

    const int enemyIdx = tileIndex(enemyTile.x, enemyTile.y, w);
    if (enemyIdx < 0 || enemyIdx >= total)
    {
      vel->velocity = { 0.f, 0.f };
      continue;
    }

    const int d0 = dist[enemyIdx];
    if (d0 <= 0)
    {
      vel->velocity = { 0.f, 0.f };
      continue;
    }

    TilePos bestNext = enemyTile;
    int bestDist = d0;

    for (const auto& d : kDirs)
    {
      const TilePos nxt{ enemyTile.x + d[0], enemyTile.y + d[1] };
      if (!canMove(*map, enemyTile, nxt)) continue;

      const int ni = tileIndex(nxt.x, nxt.y, w);
      if (ni < 0 || ni >= total) continue;

      const int dn = dist[ni];
      if (dn == -1) continue;
      if (dn >= bestDist) continue;

      if (occupied.contains(ni) && ni != enemyIdx) continue;

      bestDist = dn;
      bestNext = nxt;
    }

    if (bestNext == enemyTile)
    {
      for (const auto& d : kDirs)
      {
        const TilePos nxt{ enemyTile.x + d[0], enemyTile.y + d[1] };
        if (!canMove(*map, enemyTile, nxt)) continue;

        const int ni = tileIndex(nxt.x, nxt.y, w);
        if (ni < 0 || ni >= total) continue;
        const int dn = dist[ni];
        if (dn == -1) continue;
        if (dn >= bestDist) continue;

        bestDist = dn;
        bestNext = nxt;
      }
    }

    const sf::Vector2f target = tileCenterWorld(*map, bestNext.x, bestNext.y);
    const sf::Vector2f delta = target - pos->position;

    if (lengthSq(delta) <= (map->tileSize * 0.05f) * (map->tileSize * 0.05f))
    {
      vel->velocity = { 0.f, 0.f };
      continue;
    }

    const float len = std::hypot(delta.x, delta.y);
    if (!std::isfinite(len) || len <= SMALL_EPSILON)
    {
      vel->velocity = { 0.f, 0.f };
      continue;
    }

    const float inv = 1.f / len;
    const sf::Vector2f dir{ delta.x * inv, delta.y * inv };
    const float v = speed->speed * vel->velocityMultiplier;
    vel->velocity = { dir.x * v, dir.y * v };
  }
}