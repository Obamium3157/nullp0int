//
// Created by obamium3157 on 13.11.2025.
//

#ifndef NULLP0INT_ENTITY_H
#define NULLP0INT_ENTITY_H
#include <cstdint>

namespace ecs
{

  struct PlayerTag{};
  struct WallTag{};

  using Entity = uint32_t;
  constexpr Entity INVALID_ENTITY = 0xFFFFFFFFu;
}

#endif //NULLP0INT_ENTITY_H