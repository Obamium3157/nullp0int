//
// Created by obamium3157 on 09.01.2026.
//

#ifndef NULLP0INT_PATHFINDINGPERCEPTION_H
#define NULLP0INT_PATHFINDINGPERCEPTION_H
#include <SFML/System/Vector2.hpp>

#include "../../Components.h"
#include "../../Entity.h"
#include "../../Registry.h"

namespace ecs::npc
{
  struct PerceptionResult
  {
    sf::Vector2f toPlayer{};
    sf::Vector2f toPlayerDir{};
    float distWorld = 0.f;
    float distTilesEuclid = 0.f;
    bool withinVisionRange = false;
    bool los = false;
    bool seesPlayerNow = false;
  };

  [[nodiscard]] Entity findPlayer(const Registry& registry);

  [[nodiscard]] bool hasLineOfSightWorld(
    const TilemapComponent& map,
    sf::Vector2f fromWorld,
    sf::Vector2f toWorld);

  [[nodiscard]] bool passesFovCone(
    Registry& registry,
    Entity enemyEnt,
    const EnemyComponent& enemyComp,
    sf::Vector2f toPlayerDir);

  [[nodiscard]] PerceptionResult computePerception(
    Registry& registry,
    Entity enemyEnt,
    const PositionComponent& enemyPos,
    const EnemyComponent& enemyComp,
    const TilemapComponent& map,
    const PositionComponent& playerPos);
}

#endif //NULLP0INT_PATHFINDINGPERCEPTION_H