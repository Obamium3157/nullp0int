//
// Created by obamium3157 on 14.11.2025.
//

#ifndef NULLP0INT_RAYCASTING_H
#define NULLP0INT_RAYCASTING_H

#include "../../Registry.h"

namespace ecs
{
  class RayCasting
  {
  public:
    static void rayCast(Registry& registry, Entity& player);

  private:
    static Entity findTilemapEntity(const Registry& registry);
  };
}



#endif //NULLP0INT_RAYCASTING_H