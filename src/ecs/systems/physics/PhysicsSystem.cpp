//
// Created by obamium3157 on 14.11.2025.
//

#include "PhysicsSystem.h"

#include "../../Components.h"
#include "../collision/CollisionSystem.h"

void ecs::PhysicsSystem::update(Registry &registry, const float deltaTime, const Entity tilemapEntity)
{
  for (const auto &ents = registry.entities(); const auto &e : ents)
  {
    auto* positionComp = registry.getComponent<PositionComponent>(e);
    auto* velocityComp = registry.getComponent<VelocityComponent>(e);
    auto* rotationComp = registry.getComponent<RotationComponent>(e);
    const auto* radiusComp= registry.getComponent<RadiusComponent>(e);

    if (!positionComp || !velocityComp || !radiusComp) continue;

    const float radius = radiusComp->radius;
    const sf::Vector2f velocity = velocityComp->velocity;
    const sf::Vector2f currentPos = positionComp->position;
    const sf::Vector2f desiredPos = currentPos + velocity * deltaTime;

    if (const sf::Vector2f tryPosX{ desiredPos.x, currentPos.y };
      !CollisionSystem::checkWallCollision(registry, tryPosX, radius, tilemapEntity))
    {
      positionComp->position.x = tryPosX.x;
    }
    else
    {
      velocityComp->velocity.x = 0.f;
    }

    if (const sf::Vector2f tryPosY{positionComp->position.x, desiredPos.y};
      !CollisionSystem::checkWallCollision(registry, tryPosY, radius, tilemapEntity))
    {
      positionComp->position.y = tryPosY.y;
    }
    else
    {
      velocityComp->velocity.y = 0.f;
    }

    if (const auto* rotationVel = registry.getComponent<RotationVelocityComponent>(e); rotationComp && rotationVel)
    {
      rotationComp->angle += rotationVel->rotationVelocity * deltaTime;
    }
  }
}
