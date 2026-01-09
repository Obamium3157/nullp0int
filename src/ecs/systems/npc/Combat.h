//
// Created by obamium3157 on 09.01.2026.
//

#ifndef NULLP0INT_COMBAT_H
#define NULLP0INT_COMBAT_H

#include "PathfindingPerception.h"

namespace ecs::npc
{
  [[nodiscard]] bool canStartAttack(const EnemyComponent& enemy);

  void applyCooldown(EnemyComponent& enemy);

  [[nodiscard]] bool updateCombat(
    Registry& registry,
    Entity tilemapEntity,
    Entity enemyEntity,
    sf::Vector2f enemyWorldPos,
    float enemyRadius,
    EnemyComponent& enemy,
    SpriteComponent& sprite,
    VelocityComponent& vel,
    HealthComponent& playerHealth,
    const PerceptionResult& perception,
    float tileSize,
    float dtSeconds
  );
}

#endif //NULLP0INT_COMBAT_H