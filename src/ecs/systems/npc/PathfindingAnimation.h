//
// Created by obamium3157 on 09.01.2026.
//

#ifndef NULLP0INT_PATHFINDINGANIMATION_H
#define NULLP0INT_PATHFINDINGANIMATION_H
#include "../../Components.h"

namespace ecs::npc
{
  void setVelocityStop(VelocityComponent& vel);

  void setAnimation(
    SpriteComponent& sprite,
    const std::vector<std::string>& frames,
    float frameTime,
    bool loop,
    bool playing
  );

  void enterPassive(EnemyComponent& enemy, SpriteComponent& sprite);
  void enterMoving(EnemyComponent& enemy, SpriteComponent& sprite);
  void enterAttacking(EnemyComponent& enemy, SpriteComponent& sprite);
}

#endif //NULLP0INT_PATHFINDINGANIMATION_H