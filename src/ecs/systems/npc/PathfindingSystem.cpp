//
// Created by obamium3157 on 27.12.2025.
//

#include "PathfindingSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <unordered_set>
#include <vector>

#include <SFML/System/Vector2.hpp>

#include "../../Components.h"
#include "../../Entity.h"
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

    [[nodiscard]] int idx(const int x, const int y) const
    {
      return x + y * w;
    }

    [[nodiscard]] bool blocked(const int x, const int y) const
    {
      if (!map) return true;
      return map->isWall(x, y);
    }

    [[nodiscard]] sf::Vector2f tileCenterWorld(const int tx, const int ty) const
    {
      return sf::Vector2f{
        (static_cast<float>(tx) + 0.5f) * tileSize,
        (static_cast<float>(ty) + 0.5f) * tileSize
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

  [[nodiscard]] sf::Vector2f normalizedOrZero(const sf::Vector2f v)
  {
    const float len = std::hypot(v.x, v.y);
    if (len <= 1e-6f) return {0.f, 0.f};
    return {v.x / len, v.y / len};
  }

  [[nodiscard]] float clamp01(const float x)
  {
    if (x < 0.f) return 0.f;
    if (x > 1.f) return 1.f;
    return x;
  }

  [[nodiscard]] bool hasLineOfSightWorld(const ecs::TilemapComponent& map, const sf::Vector2f fromWorld, const sf::Vector2f toWorld)
  {
    const auto tileSize = static_cast<double>(map.tileSize);
    if (tileSize <= 0.0) return false;

    const sf::Vector2f dWorld = toWorld - fromWorld;
    const double distWorld = std::hypot(static_cast<double>(dWorld.x), static_cast<double>(dWorld.y));
    if (!std::isfinite(distWorld) || distWorld <= BIG_EPSILON) return true;

    const double dirX = static_cast<double>(dWorld.x) / distWorld;
    const double dirY = static_cast<double>(dWorld.y) / distWorld;

    const double rayX = static_cast<double>(fromWorld.x) / tileSize + dirX * BIG_EPSILON;
    const double rayY = static_cast<double>(fromWorld.y) / tileSize + dirY * BIG_EPSILON;

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

    while (traveledTiles <= maxDistTiles + BIG_EPSILON)
    {
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

  [[nodiscard]] bool passesFovCone(
    ecs::Registry& registry,
    const ecs::Entity enemyEnt,
    const ecs::EnemyComponent& enemyComp,
    const sf::Vector2f toPlayerDir
  )
  {
    const auto* rot = registry.getComponent<ecs::RotationComponent>(enemyEnt);
    if (!rot) return true;

    const float fov = enemyComp.fovDegrees;
    if (fov >= 359.9f) return true;

    const float enemyAngleRad = radiansFromDegrees(rot->angle);
    const sf::Vector2f forward{ std::cos(enemyAngleRad), std::sin(enemyAngleRad) };

    const float dot = forward.x * toPlayerDir.x + forward.y * toPlayerDir.y;
    const float dotClamped = clamp01((dot + 1.f) * 0.5f) * 2.f - 1.f;
    const float angle = std::acos(dotClamped);

    const float halfFovRad = radiansFromDegrees(fov * 0.5f);
    return angle <= halfFovRad;
  }

  void setVelocityStop(ecs::VelocityComponent& vel)
  {
    vel.velocity = {0.f, 0.f};
  }

  void setAnimation(ecs::SpriteComponent& sprite, const std::vector<std::string>& frames, const float frameTime, const bool loop, const bool playing)
  {
    sprite.textureFrames = frames;
    sprite.frameTime = frameTime;
    sprite.loop = loop;
    sprite.playing = playing;
    sprite.currentFrame = 0;
    sprite.frameAccumulator = 0.f;
  }

  void enterPassive(ecs::EnemyComponent& enemy, ecs::SpriteComponent& sprite)
  {
    enemy.state = ecs::EnemyState::PASSIVE;

    if (const std::vector<std::string>& idle = (!enemy.idleFrames.empty()) ? enemy.idleFrames : enemy.walkFrames; !idle.empty())
    {
      setAnimation(sprite, idle, enemy.walkFrameTime, true, false);
    }
    else
    {
      sprite.playing = false;
      sprite.currentFrame = 0;
      sprite.frameAccumulator = 0.f;
    }
  }

  void enterMoving(ecs::EnemyComponent& enemy, ecs::SpriteComponent& sprite)
  {
    enemy.state = ecs::EnemyState::MOVING;

    if (!enemy.walkFrames.empty())
    {
      setAnimation(sprite, enemy.walkFrames, enemy.walkFrameTime, true, true);
    }
  }

  void enterAttacking(ecs::EnemyComponent& enemy, ecs::SpriteComponent& sprite)
  {
    enemy.state = ecs::EnemyState::ATTACKING;
    enemy.attackDamageApplied = false;
    enemy.attackApplyFrame = 0;

    if (!enemy.attackFrames.empty())
    {
      setAnimation(sprite, enemy.attackFrames, enemy.attackFrameTime, false, true);
      enemy.attackApplyFrame = enemy.attackFrames.size() / 2;
    }
    else
    {
      sprite.playing = false;
      enemy.attackApplyFrame = 0;
    }
  }

  [[nodiscard]] bool canStartAttack(const ecs::EnemyComponent& enemy)
  {
    return enemy.cooldownRemainingSeconds <= 0.f;
  }

  void applyCooldown(ecs::EnemyComponent& enemy)
  {
    enemy.cooldownRemainingSeconds = enemy.attackCooldownSeconds;
  }

  [[nodiscard]] int getDistanceTiles(const std::vector<int>& dist, const Grid& g, const sf::Vector2i tile)
  {
    if (!g.inBounds(tile.x, tile.y)) return -1;
    const int d = dist[g.idx(tile.x, tile.y)];
    return d;
  }

  [[nodiscard]] sf::Vector2i pickNextTileToward(
    const Grid& g,
    const std::vector<int>& dist,
    const std::unordered_set<int>& occupied,
    const sf::Vector2i curTile,
    const sf::Vector2i playerTile
  )
  {
    static const sf::Vector2i kDirs8[8] = {
      { 1,  0}, {-1,  0}, { 0,  1}, { 0, -1},
      { 1,  1}, { 1, -1}, {-1,  1}, {-1, -1}
    };

    if (!g.inBounds(curTile.x, curTile.y)) return curTile;

    const int curIndex = g.idx(curTile.x, curTile.y);
    const int curDist = dist[curIndex];
    if (curDist < 0) return curTile;

    sf::Vector2i best = curTile;
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
      best = {nx, ny};
    }

    return best;
  }

  [[nodiscard]] sf::Vector2i pickNextTileAway(
    const Grid& g,
    const std::vector<int>& dist,
    const std::unordered_set<int>& occupied,
    const sf::Vector2i curTile
  )
  {
    static const sf::Vector2i kDirs8[8] = {
      { 1,  0}, {-1,  0}, { 0,  1}, { 0, -1},
      { 1,  1}, { 1, -1}, {-1,  1}, {-1, -1}
    };

    if (!g.inBounds(curTile.x, curTile.y)) return curTile;
    const int curIndex = g.idx(curTile.x, curTile.y);
    const int curDist = dist[curIndex];
    if (curDist < 0) return curTile;

    sf::Vector2i best = curTile;
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
      if (nd <= bestDist) continue;

      if (occupied.contains(nIndex) && nIndex != curIndex) continue;

      bestDist = nd;
      best = {nx, ny};
    }

    return best;
  }

  [[nodiscard]] sf::Vector2i pickOrbitTile(
    const Grid& g,
    const std::vector<int>& dist,
    const std::unordered_set<int>& occupied,
    const sf::Vector2i curTile,
    const sf::Vector2i playerTile,
    const float desiredRangeTiles,
    const float toleranceTiles,
    const sf::Vector2f toPlayerDir,
    const bool clockwise
  )
  {
    static const sf::Vector2i kDirs8[8] = {
      { 1,  0}, {-1,  0}, { 0,  1}, { 0, -1},
      { 1,  1}, { 1, -1}, {-1,  1}, {-1, -1}
    };

    if (!g.inBounds(curTile.x, curTile.y)) return curTile;

    const int curIndex = g.idx(curTile.x, curTile.y);
    const int curDist = dist[curIndex];
    if (curDist < 0) return curTile;

    sf::Vector2i best = curTile;
    float bestScore = std::numeric_limits<float>::infinity();

    const sf::Vector2f tangent = clockwise
      ? sf::Vector2f{-toPlayerDir.y, toPlayerDir.x}
      : sf::Vector2f{toPlayerDir.y, -toPlayerDir.x};

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

      if (!(nx == playerTile.x && ny == playerTile.y))
      {
        if (occupied.contains(nIndex) && nIndex != curIndex) continue;
      }

      const float rangeErr = std::abs(static_cast<float>(nd) - desiredRangeTiles);
      if (rangeErr > toleranceTiles * 2.0f) continue;

      const sf::Vector2f stepDir = normalizedOrZero(sf::Vector2f{static_cast<float>(d.x), static_cast<float>(d.y)});
      const float tangential = 1.f - clamp01((stepDir.x * tangent.x + stepDir.y * tangent.y + 1.f) * 0.5f);

      const float orbitPenalty = (nd == curDist) ? 0.f : 0.15f;
      const float driftPenalty = std::abs(static_cast<float>(nd) - static_cast<float>(dist[curIndex])) * 0.15f;

      if (const float score = rangeErr * 1.0f + tangential * 0.7f + orbitPenalty + driftPenalty; score < bestScore)
      {
        bestScore = score;
        best = {nx, ny};
      }
    }

    (void)playerTile;
    return best;
  }
}

void ecs::PathfindingSystem::update(Registry& registry, const Entity tilemapEntity, const float dt)
{
  const Entity player = findPlayer(registry);
  if (player == INVALID_ENTITY) return;

  const auto* playerPos = registry.getComponent<PositionComponent>(player);
  auto* playerHealth = registry.getComponent<HealthComponent>(player);
  if (!playerPos || !playerHealth) return;

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

  struct DistanceFieldCache
  {
    int w = 0;
    int h = 0;
    sf::Vector2i playerTile{ -999, -999 };
    std::vector<int> dist;
    bool valid = false;
  };

  static DistanceFieldCache cache;

  const bool needRebuild =
    !cache.valid ||
    cache.w != g.w ||
    cache.h != g.h ||
    cache.playerTile != playerTile;

  if (needRebuild)
  {
    cache.w = g.w;
    cache.h = g.h;
    cache.playerTile = playerTile;
    cache.dist.assign(static_cast<std::size_t>(g.w) * static_cast<std::size_t>(g.h), -1);
    cache.valid = true;

    if (!g.blocked(playerTile.x, playerTile.y))
    {
      static const sf::Vector2i kDirs8[8] = {
        { 1,  0}, {-1,  0}, { 0,  1}, { 0, -1},
        { 1,  1}, { 1, -1}, {-1,  1}, {-1, -1}
      };

      std::queue<sf::Vector2i> q;
      cache.dist[g.idx(playerTile.x, playerTile.y)] = 0;
      q.push(playerTile);

      while (!q.empty())
      {
        const sf::Vector2i cur = q.front();
        q.pop();

        const int curD = cache.dist[g.idx(cur.x, cur.y)];
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
          if (cache.dist[nIndex] != -1) continue;

          cache.dist[nIndex] = curD + 1;
          q.emplace(nx, ny);
        }
      }
    }
  }

  std::unordered_set<int> occupied;
  occupied.reserve(registry.entities().size());
  for (const auto& e : registry.entities())
  {
    if (!registry.hasComponent<EnemyTag>(e)) continue;
    const auto* pos = registry.getComponent<PositionComponent>(e);
    const auto* enemy = registry.getComponent<EnemyComponent>(e);
    if (!pos || !enemy) continue;
    if (!enemy->hasSeenPlayer) continue;
    const sf::Vector2i t = tilemap->worldToTile(pos->position);
    if (!g.inBounds(t.x, t.y)) continue;
    occupied.insert(g.idx(t.x, t.y));
  }

  const float dtSafe = std::max(0.f, dt);

  for (const auto& e : registry.entities())
  {
    if (!registry.hasComponent<EnemyTag>(e)) continue;

    auto* pos = registry.getComponent<PositionComponent>(e);
    auto* vel = registry.getComponent<VelocityComponent>(e);
    const auto* speed = registry.getComponent<SpeedComponent>(e);
    auto* enemy = registry.getComponent<EnemyComponent>(e);
    auto* sprite = registry.getComponent<SpriteComponent>(e);

    if (!pos || !vel || !speed || !enemy || !sprite) continue;

    enemy->cooldownRemainingSeconds = std::max(0.f, enemy->cooldownRemainingSeconds - dtSafe);

    const sf::Vector2f toPlayer = playerPos->position - pos->position;
    const float distWorld = std::hypot(toPlayer.x, toPlayer.y);
    const float visionRangeWorld = enemy->visionRangeTiles * g.tileSize;

    const bool withinVisionRange = std::isfinite(distWorld) && distWorld <= visionRangeWorld;

    const bool losNow = hasLineOfSightWorld(*tilemap, pos->position, playerPos->position);

    const sf::Vector2i enemyTile = tilemap->worldToTile(pos->position);
    const bool enemyTileValid = g.inBounds(enemyTile.x, enemyTile.y);

    bool seesPlayerNow = false;
    if (withinVisionRange && losNow)
    {
      const sf::Vector2f toPlayerDir = normalizedOrZero(toPlayer);
      seesPlayerNow = passesFovCone(registry, e, *enemy, toPlayerDir);
    }

    if (!enemy->hasSeenPlayer && seesPlayerNow)
    {
      enemy->hasSeenPlayer = true;
      enterMoving(*enemy, *sprite);
    }

    if (!enemy->hasSeenPlayer)
    {
      if (enemy->state != EnemyState::PASSIVE) enterPassive(*enemy, *sprite);
      setVelocityStop(*vel);
      continue;
    }

    const float tileSize = g.tileSize;
    const float meleeRangeWorld = enemy->meleeAttackRangeTiles * tileSize;
    const float rangedAttackRangeWorld = enemy->rangedAttackRangeTiles * tileSize;

    if (enemy->state == EnemyState::ATTACKING)
    {
      setVelocityStop(*vel);

      if (!enemy->attackDamageApplied)
      {
        if (const std::size_t applyFrame = enemy->attackApplyFrame;
          (!sprite->playing) || (sprite->currentFrame >= applyFrame))
        {
          switch (enemy->cls)
          {
            case EnemyClass::MELEE:
              if (distWorld <= meleeRangeWorld)
              {
                playerHealth->current = std::max(0.f, playerHealth->current - enemy->meleeAttackDamage);
              }
              break;

            case EnemyClass::RANGE:
            case EnemyClass::SUPPORT:
              if (distWorld <= rangedAttackRangeWorld && losNow)
              {
                playerHealth->current = std::max(0.f, playerHealth->current - enemy->rangedAttackDamage);
              }
              break;
          }

          enemy->attackDamageApplied = true;
        }
      }

      if (!sprite->playing)
      {
        applyCooldown(*enemy);

        const bool meleeInRange = distWorld <= meleeRangeWorld;
        const bool rangedCanShoot = (distWorld <= rangedAttackRangeWorld && losNow);

        if (enemy->cls == EnemyClass::MELEE)
        {
          if (meleeInRange && canStartAttack(*enemy))
          {
            enterAttacking(*enemy, *sprite);
          }
          else
          {
            enterMoving(*enemy, *sprite);
          }
        }
        else
        {
          if (rangedCanShoot && canStartAttack(*enemy))
          {
            enterAttacking(*enemy, *sprite);
          }
          else
          {
            enterMoving(*enemy, *sprite);
          }
        }
      }

      continue;
    }

    if (enemy->state != EnemyState::MOVING)
    {
      enterMoving(*enemy, *sprite);
    }

    const bool meleeInRange = distWorld <= meleeRangeWorld;
    const bool rangedCanShoot = distWorld <= rangedAttackRangeWorld && losNow;

    if (enemy->cls == EnemyClass::MELEE)
    {
      if (meleeInRange && canStartAttack(*enemy))
      {
        enterAttacking(*enemy, *sprite);
        setVelocityStop(*vel);
        continue;
      }
    }
    else
    {
      if (rangedCanShoot && canStartAttack(*enemy))
      {
        enterAttacking(*enemy, *sprite);
        setVelocityStop(*vel);
        continue;
      }
    }

    const float v = speed->speed * vel->velocityMultiplier;

    const sf::Vector2f toPlayerDir = normalizedOrZero(toPlayer);

    if (enemy->cls == EnemyClass::MELEE)
    {
      if (losNow)
      {
        vel->velocity = { toPlayerDir.x * v, toPlayerDir.y * v };
        continue;
      }

      const sf::Vector2i curTile = enemyTile;
      if (!enemyTileValid)
      {
        setVelocityStop(*vel);
        continue;
      }

      const sf::Vector2i nextTile = pickNextTileToward(g, cache.dist, occupied, curTile, playerTile);
      if (nextTile == curTile)
      {
        setVelocityStop(*vel);
        continue;
      }

      const sf::Vector2f target = g.tileCenterWorld(nextTile.x, nextTile.y);
      const sf::Vector2f dir = normalizedOrZero(target - pos->position);
      vel->velocity = { dir.x * v, dir.y * v };
      continue;
    }

    const float desired = enemy->rangedPreferredRangeTiles;
    const float tol = enemy->rangedRangeToleranceTiles;

    int distTiles = -1;
    if (enemyTileValid)
    {
      distTiles = getDistanceTiles(cache.dist, g, enemyTile);
    }

    if (distTiles < 0)
    {
      distTiles = static_cast<int>(std::round(distWorld / tileSize));
    }

    const auto distTilesF = static_cast<float>(distTiles);

    const bool tooFar = distTilesF > desired + tol;
    const bool tooClose = distTilesF < desired - tol;

    if (losNow)
    {
      if (tooFar)
      {
        vel->velocity = { toPlayerDir.x * v, toPlayerDir.y * v };
        continue;
      }

      if (tooClose)
      {
        vel->velocity = { -toPlayerDir.x * v, -toPlayerDir.y * v };
        continue;
      }

      const sf::Vector2f perp = (entityIndex(e) % 2 == 0)
        ? sf::Vector2f{-toPlayerDir.y, toPlayerDir.x}
        : sf::Vector2f{toPlayerDir.y, -toPlayerDir.x};

      vel->velocity = { perp.x * v, perp.y * v };
      continue;
    }

    if (!enemyTileValid)
    {
      setVelocityStop(*vel);
      continue;
    }

    const bool clockwise = (entityIndex(e) % 2 == 0);
    sf::Vector2i nextTile = enemyTile;

    if (tooFar)
    {
      nextTile = pickNextTileToward(g, cache.dist, occupied, enemyTile, playerTile);
    }
    else if (tooClose)
    {
      nextTile = pickNextTileAway(g, cache.dist, occupied, enemyTile);
    }
    else
    {
      nextTile = pickOrbitTile(g, cache.dist, occupied, enemyTile, playerTile, desired, tol, toPlayerDir, clockwise);
    }

    if (nextTile == enemyTile)
    {
      setVelocityStop(*vel);
      continue;
    }

    const sf::Vector2f target = g.tileCenterWorld(nextTile.x, nextTile.y);
    const sf::Vector2f dir = normalizedOrZero(target - pos->position);
    vel->velocity = { dir.x * v, dir.y * v };
  }
}
