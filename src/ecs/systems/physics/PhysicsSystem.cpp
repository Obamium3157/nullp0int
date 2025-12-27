//
// Created by obamium3157 on 14.11.2025.
//

#include "PhysicsSystem.h"

#include <cmath>
#include "../../Components.h"
#include "../collision/CollisionSystem.h"

#include "../../../constants.h"

void ecs::PhysicsSystem::update(Registry& registry, const float deltaTime, const Entity& m_tilemap)
{
  if (deltaTime <= 0.f) return;

  for (const auto &ents = registry.entities(); const auto &e : ents)
  {
    if (registry.hasComponent<ProjectileTag>(e)) continue;
    if (!registry.hasComponent<PositionComponent>(e) || !registry.hasComponent<VelocityComponent>(e))
    {
      continue;
    }

    auto* posComp = registry.getComponent<PositionComponent>(e);
    const auto* velComp = registry.getComponent<VelocityComponent>(e);
    if (!posComp || !velComp) continue;

    const auto* radiusComp = registry.getComponent<RadiusComponent>(e);
    const auto* rotVelComp = registry.getComponent<RotationVelocityComponent>(e);

    if (auto* rotComp = registry.getComponent<RotationComponent>(e); rotComp && rotVelComp)
    {
      rotComp->angle += rotVelComp->rotationVelocity * deltaTime;
      if (rotComp->angle >= 360.f) rotComp->angle = std::fmod(rotComp->angle, 360.f);
      if (rotComp->angle < 0.f) rotComp->angle = std::fmod(rotComp->angle, 360.f) + 360.f;
    }

    const sf::Vector2f desiredMove = velComp->velocity * deltaTime;

    if (std::abs(desiredMove.x) <= SMALL_EPSILON && std::abs(desiredMove.y) <= SMALL_EPSILON) continue;

    const float radius = radiusComp ? radiusComp->radius : 0.f;

    sf::Vector2f newPosX = posComp->position;
    newPosX.x += desiredMove.x;

    bool collideX = false;
    if (radius > 0.f)
    {
      if (CollisionSystem::checkWallCollision(registry, newPosX, radius, m_tilemap))
      {
        collideX = true;
      }
      else if (CollisionSystem::checkEntityCollision(registry, newPosX, radius, e))
      {
        collideX = true;
      }
    }
    else
    {
      if (CollisionSystem::checkWallCollision(registry, newPosX, 0.f, m_tilemap))
      {
        collideX = true;
      }
    }

    if (!collideX)
    {
      posComp->position.x = newPosX.x;
    }

    sf::Vector2f newPosY = posComp->position;
    newPosY.y += desiredMove.y;

    bool collideY = false;
    if (radius > 0.f)
    {
      if (CollisionSystem::checkWallCollision(registry, newPosY, radius, m_tilemap))
      {
        collideY = true;
      }
      else if (CollisionSystem::checkEntityCollision(registry, newPosY, radius, e))
      {
        collideY = true;
      }
    }
    else
    {
      if (CollisionSystem::checkWallCollision(registry, newPosY, 0.f, m_tilemap))
      {
        collideY = true;
      }
    }

    if (!collideY)
    {
      posComp->position.y = newPosY.y;
    }
  }
}
