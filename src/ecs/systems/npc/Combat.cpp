//
// Created by obamium3157 on 09.01.2026.
//

#include "Combat.h"

#include <algorithm>

#include "PathfindingAnimation.h"

namespace ecs::npc
{
  bool canStartAttack(const EnemyComponent &enemy)
  {
    return enemy.cooldownRemainingSeconds <= 0.f;
  }

  void applyCooldown(EnemyComponent &enemy)
  {
    enemy.cooldownRemainingSeconds = enemy.attackCooldownSeconds;
  }

  static void applyDamage(
    const EnemyComponent& enemy,
    HealthComponent& playerHealth,
    const PerceptionResult& perception,
    const float meleeRangeWorld,
    const float rangedAttackRangeWorld
  )
  {
    switch (enemy.cls)
    {
      case EnemyClass::MELEE:
        if (perception.distWorld <= meleeRangeWorld)
        {
          playerHealth.current = std::max(0.f, playerHealth.current - enemy.meleeAttackDamage);
        }
        break;

      case EnemyClass::RANGE:
      case EnemyClass::SUPPORT:
        if (perception.distWorld <= rangedAttackRangeWorld && perception.los)
        {
          playerHealth.current = std::max(0.f, playerHealth.current - enemy.rangedAttackDamage);
        }
        break;
    }
  }

  bool updateCombat(
    EnemyComponent& enemy,
    SpriteComponent& sprite,
    VelocityComponent& vel,
    HealthComponent& playerHealth,
    const PerceptionResult& perception,
    const float tileSize
  )
  {
    const float meleeRangeWorld = enemy.meleeAttackRangeTiles * tileSize;
    const float rangedAttackRangeWorld = enemy.rangedAttackRangeTiles * tileSize;

    if (enemy.state == EnemyState::ATTACKING)
    {
      setVelocityStop(vel);

      if (!enemy.attackDamageApplied)
      {
        const std::size_t applyFrame = enemy.attackApplyFrame;
        if ((!sprite.playing) || (sprite.currentFrame >= applyFrame))
        {
          applyDamage(enemy, playerHealth, perception, meleeRangeWorld, rangedAttackRangeWorld);
          enemy.attackDamageApplied = true;
        }
      }

      if (!sprite.playing)
      {
        applyCooldown(enemy);

        const bool meleeInRange = (perception.distWorld <= meleeRangeWorld);
        const bool rangedCanShoot = (perception.distWorld <= rangedAttackRangeWorld) && perception.los;

        if (enemy.cls == EnemyClass::MELEE)
        {
          if (meleeInRange && canStartAttack(enemy)) enterAttacking(enemy, sprite);
          else enterMoving(enemy, sprite);
        }
        else
        {
          if (rangedCanShoot && canStartAttack(enemy)) enterAttacking(enemy, sprite);
          else enterMoving(enemy, sprite);
        }
      }

      return true;
    }

    const bool meleeInRange = (perception.distWorld <= meleeRangeWorld);
    const bool rangedCanShoot = (perception.distWorld <= rangedAttackRangeWorld) && perception.los;

    if (enemy.cls == EnemyClass::MELEE)
    {
      if (meleeInRange && canStartAttack(enemy))
      {
        enterAttacking(enemy, sprite);
        setVelocityStop(vel);
        return true;
      }
    }
    else
    {
      if (rangedCanShoot && canStartAttack(enemy))
      {
        enterAttacking(enemy, sprite);
        setVelocityStop(vel);
        return true;
      }
    }

    return false;
  }
}