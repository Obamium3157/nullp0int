//
// Created by obamium3157 on 13.12.2025.
//

#include "ProjectileSystem.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "../../Components.h"
#include "../collision/CollisionSystem.h"

namespace
{
  [[nodiscard]] bool segmentCircleIntersection(
    const sf::Vector2f p0,
    const sf::Vector2f p1,
    const sf::Vector2f c,
    const float r,
    float& outT
  )
  {
    const sf::Vector2f d{ p1.x - p0.x, p1.y - p0.y };
    const sf::Vector2f f{ p0.x - c.x, p0.y - c.y };

    const float a = d.x * d.x + d.y * d.y;
    if (a <= 1e-8f) return false;

    const float b = 2.f * (f.x * d.x + f.y * d.y);
    const float c2 = (f.x * f.x + f.y * f.y) - r * r;

    const float disc = b * b - 4.f * a * c2;
    if (disc < 0.f) return false;

    const float s = std::sqrt(disc);
    const float inv2a = 1.f / (2.f * a);

    const float t1 = (-b - s) * inv2a;
    const float t2 = (-b + s) * inv2a;

    float t = 2.f;
    if (t1 >= 0.f && t1 <= 1.f) t = t1;
    else if (t2 >= 0.f && t2 <= 1.f) t = t2;

    if (t > 1.f) return false;

    outT = t;
    return true;
  }

  [[nodiscard]] sf::Vector2f lerp(const sf::Vector2f a, const sf::Vector2f b, const float t)
  {
    return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
  }

  void applyDamageOrKill(ecs::Registry& registry, const ecs::Entity target, const float dmg)
  {
    if (target == ecs::INVALID_ENTITY) return;

    if (auto* hp = registry.getComponent<ecs::HealthComponent>(target))
    {
      hp->current -= dmg;
      if (hp->current <= 0.f)
      {
        registry.destroyEntity(target);
      }
    }
  }

  void applyDamageNoKill(ecs::Registry& registry, const ecs::Entity target, const float dmg)
  {
    if (target == ecs::INVALID_ENTITY) return;
    if (auto* hp = registry.getComponent<ecs::HealthComponent>(target))
    {
      hp->current = std::max(0.f, hp->current - dmg);
    }
  }

  [[nodiscard]] float approximateWallHitT(
    ecs::Registry& registry,
    const ecs::Entity tilemapEntity,
    const sf::Vector2f p0,
    const sf::Vector2f p1,
    const float radius,
    const float tileSize
  )
  {
    const float dx = p1.x - p0.x;
    const float dy = p1.y - p0.y;
    const float dist = std::hypot(dx, dy);
    if (!std::isfinite(dist) || dist <= 1e-6f) return 2.f;

    const float stepLen = std::max(1.f, tileSize * 0.20f);
    const int steps = std::max(1, static_cast<int>(std::ceil(dist / stepLen)));

    float lastT = 0.f;
    for (int i = 1; i <= steps; ++i)
    {
      const float t = static_cast<float>(i) / static_cast<float>(steps);
      const sf::Vector2f p = lerp(p0, p1, t);

      if (ecs::CollisionSystem::checkWallCollision(registry, p, radius, tilemapEntity))
      {
        float lo = lastT;
        float hi = t;
        for (int it = 0; it < 8; ++it)
        {
          const float mid = (lo + hi) * 0.5f;
          const sf::Vector2f pm = lerp(p0, p1, mid);
          if (ecs::CollisionSystem::checkWallCollision(registry, pm, radius, tilemapEntity)) hi = mid;
          else lo = mid;
        }
        return hi;
      }

      lastT = t;
    }

    return 2.f;
  }
}

void ecs::ProjectileSystem::update(Registry& registry, const Configuration& config, const Entity tilemapEntity, const float dtSeconds)
{
  if (dtSeconds <= 0.f) return;

  std::vector<Entity> toDestroy;
  toDestroy.reserve(64);

  const auto ents = registry.entities();

  Entity player = INVALID_ENTITY;
  for (const auto& e : ents)
  {
    if (registry.hasComponent<PlayerTag>(e))
    {
      player = e;
      break;
    }
  }

  const auto* playerPos = (player != INVALID_ENTITY) ? registry.getComponent<PositionComponent>(player) : nullptr;
  const auto* playerRad = (player != INVALID_ENTITY) ? registry.getComponent<RadiusComponent>(player) : nullptr;

  for (const auto& e : ents)
  {
    if (!registry.hasComponent<ProjectileTag>(e)) continue;

    auto* pos = registry.getComponent<PositionComponent>(e);
    auto* prj = registry.getComponent<ProjectileComponent>(e);
    if (!pos || !prj) continue;

    prj->livedSeconds += dtSeconds;
    if (prj->lifeSeconds > 0.f && prj->livedSeconds >= prj->lifeSeconds)
    {
      toDestroy.push_back(e);
      continue;
    }

    const sf::Vector2f p0 = pos->position;

    const sf::Vector2f p1{
      p0.x + prj->direction.x * prj->speed * dtSeconds,
      p0.y + prj->direction.y * prj->speed * dtSeconds
    };

    const bool ownerIsPlayer = (prj->owner != INVALID_ENTITY) && registry.hasComponent<PlayerTag>(prj->owner);

    Entity hitEntity = INVALID_ENTITY;
    float hitEntityT = 2.f;

    if (ownerIsPlayer)
    {
      for (const auto& enemy : ents)
      {
        if (!registry.hasComponent<EnemyTag>(enemy)) continue;

        if (enemy == prj->owner && prj->livedSeconds < prj->ignoreOwnerSeconds) continue;
        if (enemy == prj->owner) continue;

        const auto* ep = registry.getComponent<PositionComponent>(enemy);
        const auto* er = registry.getComponent<RadiusComponent>(enemy);
        if (!ep || !er) continue;

        const float rr = er->radius + prj->radius;

        if (float t = 2.f; segmentCircleIntersection(p0, p1, ep->position, rr, t))
        {
          if (t < hitEntityT)
          {
            hitEntityT = t;
            hitEntity = enemy;
          }
        }
      }
    }
    else
    {
      if (player != INVALID_ENTITY && playerPos && playerRad)
      {
        const float rr = playerRad->radius + prj->radius;
        if (float t = 2.f; segmentCircleIntersection(p0, p1, playerPos->position, rr, t))
        {
          hitEntityT = t;
          hitEntity = player;
        }
      }
    }

    const float wallT = approximateWallHitT(registry, tilemapEntity, p0, p1, prj->radius, config.tile_size);

    prj->positionPrev = p0;

    if (hitEntity != INVALID_ENTITY && hitEntityT <= 1.f && hitEntityT < wallT)
    {
      pos->position = lerp(p0, p1, std::clamp(hitEntityT, 0.f, 1.f));

      if (ownerIsPlayer) applyDamageOrKill(registry, hitEntity, prj->damage);
      else applyDamageNoKill(registry, hitEntity, prj->damage);

      toDestroy.push_back(e);
      continue;
    }

    if (wallT <= 1.f)
    {
      pos->position = lerp(p0, p1, std::clamp(wallT, 0.f, 1.f));
      toDestroy.push_back(e);
      continue;
    }

    pos->position = p1;
  }

  for (const auto& e : toDestroy)
  {
    if (registry.isAlive(e)) registry.destroyEntity(e);
  }
}
