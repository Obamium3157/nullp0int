//
// Created by obamium3157 on 09.01.2026.
//

#include "PathfindingAnimation.h"

namespace ecs::npc
{
  void setVelocityStop(VelocityComponent& vel)
  {
    vel.velocity = {0.f, 0.f};
  }

  void setAnimation(
    SpriteComponent& sprite,
    const std::vector<std::string>& frames,
    const float frameTime,
    const bool loop,
    const bool playing
  )
  {
    sprite.textureFrames = frames;
    sprite.frameTime = frameTime;
    sprite.loop = loop;
    sprite.playing = playing;
    sprite.currentFrame = 0;
    sprite.frameAccumulator = 0.f;
  }

  void enterPassive(EnemyComponent& enemy, SpriteComponent& sprite)
  {
    enemy.state = EnemyState::PASSIVE;

    if (const std::vector<std::string>& idle = (!enemy.idleFrames.empty()) ? enemy.idleFrames : enemy.walkFrames; !idle.empty())
    {
      setAnimation(sprite, idle, enemy.walkFrameTime, true, false);
    }
    else
    {
      sprite.playing = false;
      sprite.currentFrame = 0;
      sprite.frameAccumulator = 0.f;
    }
  }

  void enterMoving(EnemyComponent& enemy, SpriteComponent& sprite)
  {
    enemy.state = EnemyState::MOVING;

    if (!enemy.walkFrames.empty())
    {
      setAnimation(sprite, enemy.walkFrames, enemy.walkFrameTime, true, true);
    }
  }

  void enterAttacking(EnemyComponent& enemy, SpriteComponent& sprite)
  {
    enemy.state = EnemyState::ATTACKING;
    enemy.attackDamageApplied = false;
    enemy.attackApplyFrame = 0;

    if (!enemy.attackFrames.empty())
    {
      setAnimation(sprite, enemy.attackFrames, enemy.attackFrameTime, false, true);
      enemy.attackApplyFrame = enemy.attackFrames.size() / 2;
    }
    else
    {
      sprite.playing = false;
      enemy.attackApplyFrame = 0;
    }
  }
}
