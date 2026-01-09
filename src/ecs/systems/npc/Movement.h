//
// Created by obamium3157 on 09.01.2026.
//

#ifndef NULLP0INT_MOVEMENT_H
#define NULLP0INT_MOVEMENT_H

#include <unordered_set>
#include <vector>

#include <SFML/System/Vector2.hpp>

#include "PathfindingTypes.h"

namespace ecs::npc
{
  [[nodiscard]] sf::Vector2i pickNextTileToward(
    const Grid& g,
    const std::vector<int>& distField,
    const std::unordered_set<int>& occupied,
    sf::Vector2i curTile,
    sf::Vector2i playerTile
  );

  [[nodiscard]] sf::Vector2i pickNextTileAway(
    const Grid& g,
    const std::vector<int>& distField,
    const std::unordered_set<int>& occupied,
    sf::Vector2i curTile
  );

  [[nodiscard]] sf::Vector2i pickOrbitTile(
    const Grid& g,
    const std::unordered_set<int>& occupied,
    sf::Vector2i curTile,
    sf::Vector2f curWorld,
    sf::Vector2f playerWorld,
    float desiredRangeTiles,
    float toleranceTiles,
    sf::Vector2f toPlayerDir,
    bool clockwise,
    const OrbitTuning& tuning = DEFAULT_ORBIT_TUNING
  );

  [[nodiscard]] sf::Vector2f perpendicularStrafeDir(sf::Vector2f toPlayerDir, bool clockwise);
}

#endif //NULLP0INT_MOVEMENT_H