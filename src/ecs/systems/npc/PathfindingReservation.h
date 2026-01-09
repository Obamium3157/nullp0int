//
// Created by obamium3157 on 09.01.2026.
//

#ifndef NULLP0INT_PATHFINDINGRESERVATION_H
#define NULLP0INT_PATHFINDINGRESERVATION_H
#include <unordered_set>
#include <SFML/System/Vector2.hpp>

#include "PathfindingTypes.h"
#include "../../Entity.h"

namespace ecs::npc
{
  struct MoveReservation
  {
    Entity entity = INVALID_ENTITY;
    sf::Vector2i fromTile{0, 0};
    sf::Vector2i intendedTile{0, 0};
    sf::Vector2i finalTile{0, 0};
    bool wantsMove = false;
  };

  void resolveMoveReservations(
    const Grid&                    g,
    const std::unordered_set<int>& initiallyOccupied,
    std::vector<MoveReservation>&  reservations
  );
}

#endif //NULLP0INT_PATHFINDINGRESERVATION_H