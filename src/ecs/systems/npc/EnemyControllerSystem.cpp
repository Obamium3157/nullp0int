//
// Created by obamium3157 on 06.12.2025.
//

#include "EnemyControllerSystem.h"
#include <cmath>
#include "../../Components.h"
#include "../../../constants.h"

void EnemyControllerSystem::update(ecs::Registry &registry)
{
  ecs::Entity player = ecs::INVALID_ENTITY;
  const auto &ents = registry.entities();
  for (const auto &e : ents)
  {
    if (registry.hasComponent<ecs::PlayerTag>(e))
    {
      player = e;
      break;
    }
  }

  if (player == ecs::INVALID_ENTITY) return;

  const auto* playerPositionComp = registry.getComponent<ecs::PositionComponent>(player);
  if (!playerPositionComp) return;

  for (const auto &e : ents)
  {
    if (!registry.hasComponent<ecs::EnemyTag>(e)) continue;

    auto* velocityComp = registry.getComponent<ecs::VelocityComponent>(e);
    const auto* speedComp = registry.getComponent<ecs::SpeedComponent>(e);
    const auto* enemyPositionComp = registry.getComponent<ecs::PositionComponent>(e);

    if (!velocityComp || !speedComp || !enemyPositionComp) continue;

    const float dx = playerPositionComp->position.x - enemyPositionComp->position.x;
    const float dy = playerPositionComp->position.y - enemyPositionComp->position.y;
    const float dist = std::hypot(dx, dy);

    if (!std::isfinite(dist) || dist <= SMALL_EPSILON)
    {
      velocityComp->velocity = {0.f, 0.f};
      continue;
    }

    const float inv = 1.0f / dist;
    const sf::Vector2f dir{ dx * inv, dy * inv };

    const float speed = speedComp->speed * velocityComp->velocityMultiplier;
    velocityComp->velocity = sf::Vector2f{ dir.x * speed, dir.y * speed };
  }
}
