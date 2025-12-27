//
// Created by obamium3157 on 27.12.2025.
//

#ifndef NULLP0INT_WEAPONSYSTEM_H
#define NULLP0INT_WEAPONSYSTEM_H
#include "../../Registry.h"
#include "../../../configuration/Configuration.h"

namespace ecs
{
  class WeaponSystem
  {
  public:
    static void update(Registry& registry, const Configuration& config, Entity tilemapEntity, Entity playerEntity, float dtSeconds);
  };
}


#endif //NULLP0INT_WEAPONSYSTEM_H