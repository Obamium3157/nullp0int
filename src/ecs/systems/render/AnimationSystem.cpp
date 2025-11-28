//
// Created by obamium3157 on 21.11.2025.
//

#include "AnimationSystem.h"
#include "../../Components.h"

void ecs::AnimationSystem::update(Registry &registry, const float dt)
{
  for (const auto e : registry.entities())
  {
    if (registry.hasComponent<SpriteComponent>(e))
    {
      auto* sc = registry.getComponent<SpriteComponent>(e);

      if (!sc) continue;
      if (!sc->playing) continue;
      if (sc->frames.empty()) continue;

      sc->frameAccumulator += dt;
      while (sc->frameAccumulator >= sc->frameTime)
      {
        sc->frameAccumulator -= sc->frameTime;
        sc->currentFrame++;
        if (sc->currentFrame >= sc->frames.size())
        {
          if (sc->loop)
          {
            sc->currentFrame = 0;
          }
          else
          {
            sc->currentFrame = sc->frames.size() - 1; sc->playing = false;
            break;
          }
        }
      }
    }
  }
}
