//
// Created by obamium3157 on 14.11.2025.
//

#ifndef NULLP0INT_PHYSICSSYSTEM_H
#define NULLP0INT_PHYSICSSYSTEM_H

#include "../../Registry.h"

namespace ecs
{
  class PhysicsSystem
  {
  public:
    static void update(Registry& registry, float deltaTime, Entity tilemapEntity);
  };
}


#endif //NULLP0INT_PHYSICSSYSTEM_H