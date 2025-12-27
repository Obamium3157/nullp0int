//
// Created by obamium3157 on 27.12.2025.
//

#ifndef NULLP0INT_PATHFINDINGSYSTEM_H
#define NULLP0INT_PATHFINDINGSYSTEM_H
#include "../../Registry.h"


namespace ecs
{
  class PathfindingSystem
  {
  public:
    static void update(Registry& registry, Entity tilemapEntity, float dt);
  };
}


#endif //NULLP0INT_PATHFINDINGSYSTEM_H