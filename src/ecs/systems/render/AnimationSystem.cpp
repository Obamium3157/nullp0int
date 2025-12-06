//
// Created by obamium3157 on 21.11.2025.
//

#include "AnimationSystem.h"
#include "../../Components.h"

void ecs::AnimationSystem::update(Registry &registry, const float dt)
{
  for (const auto e : registry.entities())
  {
    if (!registry.hasComponent<SpriteComponent>(e)) continue;

    auto* sc = registry.getComponent<SpriteComponent>(e);
    if (!sc) continue;
    if (!sc->playing) continue;

    const std::size_t framesCount = !sc->textureFrames.empty() ? sc->textureFrames.size() : sc->frames.size();
    if (framesCount == 0) continue;

    if (sc->frameTime <= 0.f) continue;

    sc->frameAccumulator += dt;
    while (sc->frameAccumulator >= sc->frameTime)
    {
      sc->frameAccumulator -= sc->frameTime;
      sc->currentFrame++;
      if (sc->currentFrame >= framesCount)
      {
        if (sc->loop)
        {
          sc->currentFrame = 0;
        }
        else
        {
          sc->currentFrame = framesCount - 1;
          sc->playing = false;
          break;
        }
      }
    }
  }
}
