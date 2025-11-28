//
// Created by obamium3157 on 21.11.2025.
//

#ifndef NULLP0INT_ANIMATION_H
#define NULLP0INT_ANIMATION_H

#pragma once
#include <functional>
#include <vector>
#include "../../Registry.h"

namespace ecs
{
  using AnimationChannel = std::function<void(Registry&, Entity, float phase)>;

  struct Animation
  {
    std::vector<AnimationChannel> channels;
    int times = 40;
    bool loop = false;

    Animation() = default;
  };
}

#endif //NULLP0INT_ANIMATION_H