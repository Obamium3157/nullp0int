//
// Created by obamium3157 on 21.11.2025.
//

#ifndef NULLP0INT_ANIMATIONSYSTEM_H
#define NULLP0INT_ANIMATIONSYSTEM_H

#include "../../Registry.h"

#pragma once


namespace ecs
{
  class AnimationSystem
  {
  public:
    static void update(Registry& registry, float dt);
  };
}

#endif //NULLP0INT_ANIMATIONSYSTEM_H