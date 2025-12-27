//
// Created by obamium3157 on 13.12.2025.
//

#ifndef NULLP0INT_PROJECTILESYSTEM_H
#define NULLP0INT_PROJECTILESYSTEM_H
#include "../../Registry.h"
#include "../../../configuration/Configuration.h"

namespace ecs
{
  class ProjectileSystem
  {
  public:
    static void update(Registry& registry, const Configuration& config, Entity tilemapEntity, float dtSeconds);
  };
}

#endif //NULLP0INT_PROJECTILESYSTEM_H
