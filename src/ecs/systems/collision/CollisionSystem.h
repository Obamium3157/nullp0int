//
// Created by obamium3157 on 14.11.2025.
//

#ifndef NULLP0INT_COLLISIONSYSTEM_H
#define NULLP0INT_COLLISIONSYSTEM_H
#include <SFML/System/Vector2.hpp>

#include "../../Registry.h"

namespace ecs
{
  class CollisionSystem
  {
  public:
    static bool checkWallCollision(Registry &registry, sf::Vector2f position, float radius, Entity tilemap);
  };
}




#endif //NULLP0INT_COLLISIONSYSTEM_H