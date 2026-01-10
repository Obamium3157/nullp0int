//
// Created by obamium3157 on 27.12.2025.
//

#include "EnemyControllerSystem.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <vector>

#include <SFML/System/Vector2.hpp>

#include "../../Components.h"
#include "../../Entity.h"

#include "../collision/CollisionSystem.h"

#include "Combat.h"
#include "Movement.h"
#include "PathfindingAnimation.h"
#include "PathfindingDistanceField.h"
#include "PathfindingPerception.h"
#include "PathfindingReservation.h"
#include "PathfindingTypes.h"

namespace
{
  [[nodiscard]] std::unordered_set<int> buildInitiallyOccupied(
    ecs::Registry& registry,
    const ecs::npc::Grid& g,
    const ecs::TilemapComponent& tilemap
  )
  {
    std::unordered_set<int> occupied;
    occupied.reserve(registry.entities().size());

    for (const auto& e : registry.entities())
    {
      if (!registry.hasComponent<ecs::EnemyTag>(e)) continue;

      const auto* pos = registry.getComponent<ecs::PositionComponent>(e);
      if (!pos) continue;

      const sf::Vector2i t = tilemap.worldToTile(pos->position);
      if (!g.inBounds(t.x, t.y)) continue;

      occupied.insert(g.idx(t.x, t.y));
    }

    return occupied;
  }
}

void ecs::EnemyControllerSystem::update(Registry& registry, const Entity tilemapEntity, const float dt)
{
  using namespace ecs::npc;

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
  if (g.w <= 0 || g.h <= 0 || g.tileSize <= 0.f) return;

  const sf::Vector2i playerTile = tilemap->worldToTile(playerPos->position);
  if (!g.inBounds(playerTile.x, playerTile.y)) return;

  static DistanceFieldCache distCache;
  distCache.rebuildIfNeeded(*tilemap, g, playerTile);

  const std::unordered_set<int> initiallyOccupied = buildInitiallyOccupied(registry, g, *tilemap);

  std::vector<MoveReservation> reservations;
  reservations.reserve(registry.entities().size());

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

    const PerceptionResult perception = computePerception(registry, e, *pos, *enemy, *tilemap, *playerPos);

    if (!enemy->hasSeenPlayer && perception.seesPlayerNow)
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

    if ((enemy->cls == EnemyClass::RANGE || enemy->cls == EnemyClass::SUPPORT) && enemy->rangedDodgeActive)
    {
      enemy->rangedDodgeTimeRemainingSeconds = std::max(0.f, enemy->rangedDodgeTimeRemainingSeconds - dtSafe);

      if (const float v = speed->speed * vel->velocityMultiplier; !(v > 0.f))
      {
        setVelocityStop(*vel);
        enemy->rangedDodgeActive = false;
        enterMoving(*enemy, *sprite);
      }
      else
      {
        const sf::Vector2f dir = normalizedOrZero(enemy->rangedDodgeWorldDir);
        const auto step = sf::Vector2f{dir.x * v, dir.y * v};

        const float r = (registry.getComponent<RadiusComponent>(e) ? registry.getComponent<RadiusComponent>(e)->radius : 0.f);
        const auto nextPos = sf::Vector2f{pos->position.x + step.x * dtSafe, pos->position.y + step.y * dtSafe};

        if (CollisionSystem::checkWallCollision(registry, nextPos, r, tilemapEntity))
        {
          enemy->rangedDodgeActive = false;
          enemy->rangedDodgeTimeRemainingSeconds = 0.f;
          enemy->cooldownRemainingSeconds = 0.f;
          setVelocityStop(*vel);
          enterMoving(*enemy, *sprite);
        }
        else if (enemy->rangedDodgeTimeRemainingSeconds > 0.f)
        {
          vel->velocity = step;
          continue;
        }
        else
        {
          enemy->rangedDodgeActive = false;
          enemy->cooldownRemainingSeconds = 0.f;
          setVelocityStop(*vel);
          enterMoving(*enemy, *sprite);
        }
      }
    }

    if (enemy->state != EnemyState::ATTACKING && enemy->state != EnemyState::MOVING)
    {
      enterMoving(*enemy, *sprite);
    }

    const float enemyRadius = (registry.getComponent<RadiusComponent>(e) ? registry.getComponent<RadiusComponent>(e)->radius : 0.f);

    if (updateCombat(registry, tilemapEntity, e, pos->position, enemyRadius, *enemy, *sprite, *vel, *playerHealth, perception, g.tileSize, dtSafe))
    {
      continue;
    }

    if (enemy->state != EnemyState::MOVING)
    {
      enterMoving(*enemy, *sprite);
    }

    const float v = speed->speed * vel->velocityMultiplier;
    if (!(v > 0.f))
    {
      setVelocityStop(*vel);
      continue;
    }

    const sf::Vector2i enemyTile = tilemap->worldToTile(pos->position);
    const bool enemyTileValid = g.inBounds(enemyTile.x, enemyTile.y);

    if (enemy->cls == EnemyClass::MELEE)
    {
      if (perception.los)
      {
        vel->velocity = { perception.toPlayerDir.x * v, perception.toPlayerDir.y * v };
        continue;
      }

      if (!enemyTileValid)
      {
        setVelocityStop(*vel);
        continue;
      }

      const sf::Vector2i intended = pickNextTileToward(g, distCache.field(), initiallyOccupied, enemyTile, playerTile);

      MoveReservation r;
      r.entity = e;
      r.fromTile = enemyTile;
      r.intendedTile = intended;
      r.wantsMove = true;
      reservations.push_back(r);

      setVelocityStop(*vel);
      continue;
    }

    if (!perception.seesPlayerNow)
    {
      if (perception.los)
      {
        vel->velocity = { perception.toPlayerDir.x * v, perception.toPlayerDir.y * v };
        continue;
      }

      if (!enemyTileValid)
      {
        setVelocityStop(*vel);
        continue;
      }

      const sf::Vector2i intended = pickNextTileToward(g, distCache.field(), initiallyOccupied, enemyTile, playerTile);

      MoveReservation r;
      r.entity = e;
      r.fromTile = enemyTile;
      r.intendedTile = intended;
      r.wantsMove = true;
      reservations.push_back(r);

      setVelocityStop(*vel);
      continue;
    }

    const float desired = enemy->rangedPreferredRangeTiles;
    const float tol = enemy->rangedRangeToleranceTiles;

    const bool tooFar = perception.distTilesEuclid > desired + tol;
    const bool tooClose = perception.distTilesEuclid < desired - tol;

    if (perception.los)
    {
      if (tooFar)
      {
        vel->velocity = { perception.toPlayerDir.x * v, perception.toPlayerDir.y * v };
        continue;
      }

      if (tooClose)
      {
        vel->velocity = { -perception.toPlayerDir.x * v, -perception.toPlayerDir.y * v };
        continue;
      }

      const bool clockwise = (entityIndex(e) % 2 == 0);
      const sf::Vector2f perp = perpendicularStrafeDir(perception.toPlayerDir, clockwise);
      vel->velocity = { perp.x * v, perp.y * v };
      continue;
    }

    if (!enemyTileValid)
    {
      setVelocityStop(*vel);
      continue;
    }

    sf::Vector2i intended = enemyTile;
    const bool clockwise = (entityIndex(e) % 2 == 0);

    if (tooFar)
    {
      intended = pickNextTileToward(g, distCache.field(), initiallyOccupied, enemyTile, playerTile);
    }
    else if (tooClose)
    {
      intended = pickNextTileAway(g, distCache.field(), initiallyOccupied, enemyTile);
    }
    else
    {
      intended = pickOrbitTile(
        g,
        initiallyOccupied,
        enemyTile,
        pos->position,
        playerPos->position,
        desired,
        tol,
        perception.toPlayerDir,
        clockwise,
        DEFAULT_ORBIT_TUNING
      );
    }

    MoveReservation r;
    r.entity = e;
    r.fromTile = enemyTile;
    r.intendedTile = intended;
    r.wantsMove = true;
    reservations.push_back(r);

    setVelocityStop(*vel);
  }

  resolveMoveReservations(g, initiallyOccupied, reservations);

  for (const auto& r : reservations)
  {
    if (!r.wantsMove) continue;

    auto* pos = registry.getComponent<PositionComponent>(r.entity);
    auto* vel = registry.getComponent<VelocityComponent>(r.entity);
    const auto* speed = registry.getComponent<SpeedComponent>(r.entity);
    const auto* enemy = registry.getComponent<EnemyComponent>(r.entity);

    if (!pos || !vel || !speed || !enemy) continue;
    if (enemy->state != EnemyState::MOVING) continue;

    const float v = speed->speed * vel->velocityMultiplier;
    if (!(v > 0.f))
    {
      setVelocityStop(*vel);
      continue;
    }

    if (r.finalTile == r.fromTile)
    {
      setVelocityStop(*vel);
      continue;
    }

    const sf::Vector2f target = g.tileCenterWorld(r.finalTile.x, r.finalTile.y);
    const sf::Vector2f dir = normalizedOrZero(target - pos->position);
    vel->velocity = { dir.x * v, dir.y * v };
  }
}
