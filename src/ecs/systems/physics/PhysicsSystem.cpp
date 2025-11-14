//
// Created by obamium3157 on 14.11.2025.
//

#include "PhysicsSystem.h"

#include "../../Components.h"

void ecs::PhysicsSystem::update(Registry &registry, const float deltaTime)
{
  for (const auto &ents = registry.entities(); const auto &e : ents)
  {
    auto* pos = registry.getComponent<PositionComponent>(e);
    auto* rotation = registry.getComponent<RotationComponent>(e);

    if (const auto* vel = registry.getComponent<VelocityComponent>(e); pos && vel)
    {
      pos->position += vel->velocity * deltaTime;
    }

    if (const auto* rotationVel = registry.getComponent<RotationVelocityComponent>(e); rotation && rotationVel)
    {
      rotation->angle += rotationVel->rotationVelocity * deltaTime;
    }
  }
}
