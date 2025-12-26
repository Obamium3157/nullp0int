//
// Created by obamium3157 on 13.11.2025.
//

#ifndef NULLP0INT_ENTITY_H
#define NULLP0INT_ENTITY_H
#include <cstdint>
#include <limits>

namespace ecs
{
  using Entity = uint64_t;

  using EntityIndex = uint32_t;
  using EntityGeneration = uint32_t;

  constexpr Entity INVALID_ENTITY = (std::numeric_limits<Entity>::max)();

  constexpr Entity makeEntity(const EntityIndex idx, const EntityGeneration gen)
  {
    return (static_cast<Entity>(gen) << 32) | static_cast<Entity>(idx);
  }

  constexpr EntityIndex entityIndex(Entity e)
  {
    return static_cast<EntityIndex>(e & 0xFFFFFFFFull);
  }

  constexpr EntityGeneration entityGeneration(Entity e)
  {
    return static_cast<EntityGeneration>((e >> 32) & 0xFFFFFFFFull);
  }
}

#endif //NULLP0INT_ENTITY_H