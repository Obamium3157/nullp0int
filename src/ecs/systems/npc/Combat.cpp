//
// Created by obamium3157 on 09.01.2026.
//

#include "Combat.h"

#include <algorithm>

#include "PathfindingAnimation.h"
#include "../collision/CollisionSystem.h"

namespace ecs::npc
{
  bool canStartAttack(const EnemyComponent& enemy)
  {
    return enemy.cooldownRemainingSeconds <= 0.f;
  }

  void applyCooldown(EnemyComponent& enemy)
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
        if (perception.distWorld <= rangedAttackRangeWorld && perception.seesPlayerNow)
        {
          playerHealth.current = std::max(0.f, playerHealth.current - enemy.rangedAttackDamage);
        }
        break;

      case EnemyClass::SUPPORT:
        break;
    }
  }

  struct ProjectileParams
  {
    float speed = 0.f;
    float radius = 0.f;
    float lifeSeconds = 0.f;
    float visualSizeTiles = 0.25f;
    float spriteScale = 1.f;
    float heightShift = 0.12f;
  };

  [[nodiscard]] static ProjectileParams supportProjectileParams()
  {
    ProjectileParams p;
    p.speed = 1500.f;
    p.radius = 5.f;
    p.lifeSeconds = 5.f;
    p.visualSizeTiles = 0.22f;
    p.spriteScale = 1.f;
    p.heightShift = 0.15f;
    return p;
  }

  [[nodiscard]] static Entity spawnSupportProjectile(
    Registry& registry,
    const Entity tilemapEntity,
    const Entity owner,
    const sf::Vector2f ownerPos,
    const float ownerRadius,
    const sf::Vector2f dir,
    const ProjectileParams& params,
    const float damage,
    const float tileSize
  )
  {
    const float pr = std::max(0.5f, params.radius);
    const float spawnBaseOffset = ownerRadius + pr + 6.f;

    sf::Vector2f spawnPos{ ownerPos.x + dir.x * spawnBaseOffset, ownerPos.y + dir.y * spawnBaseOffset };

    const float nudge = tileSize * 0.20f;
    for (int i = 0; i < 8; ++i)
    {
      if (!ecs::CollisionSystem::checkWallCollision(registry, spawnPos, pr, tilemapEntity)) break;
      spawnPos.x += dir.x * nudge;
      spawnPos.y += dir.y * nudge;
    }

    if (ecs::CollisionSystem::checkWallCollision(registry, spawnPos, pr, tilemapEntity))
    {
      return INVALID_ENTITY;
    }

    const Entity proj = registry.createEntity();
    registry.addComponent<PositionComponent>(proj, PositionComponent{spawnPos});
    registry.addComponent<ProjectileTag>(proj, ProjectileTag{});

    ProjectileComponent pc;
    pc.owner = owner;
    pc.positionPrev = spawnPos;
    pc.direction = dir;
    pc.speed = params.speed;
    pc.damage = damage;
    pc.radius = pr;
    pc.lifeSeconds = params.lifeSeconds;
    pc.livedSeconds = 0.f;
    pc.textureId = "support_projectile";
    pc.spriteScale = params.spriteScale;
    pc.heightShift = params.heightShift;
    pc.visualSizeTiles = params.visualSizeTiles;
    pc.ignoreOwnerSeconds = 0.06f;

    registry.addComponent<ProjectileComponent>(proj, pc);
    return proj;
  }

  bool updateCombat(
    Registry& registry,
    const Entity tilemapEntity,
    const Entity enemyEntity,
    const sf::Vector2f enemyWorldPos,
    const float enemyRadius,
    EnemyComponent& enemy,
    SpriteComponent& sprite,
    VelocityComponent& vel,
    HealthComponent& playerHealth,
    const PerceptionResult& perception,
    const float tileSize,
    const float dtSeconds
  )
  {

    const float meleeRangeWorld = enemy.meleeAttackRangeTiles * tileSize;
    const float rangedAttackRangeWorld = enemy.rangedAttackRangeTiles * tileSize;

    constexpr float kSupportBurstIntervalSeconds = 0.08f;
    constexpr int kSupportBurstShots = 3;

    if (enemy.state == EnemyState::ATTACKING)
    {
      setVelocityStop(vel);

      if (enemy.cls == EnemyClass::SUPPORT)
      {
        if (!perception.seesPlayerNow || perception.distWorld > rangedAttackRangeWorld)
        {
          enemy.supportBurstShotsRemaining = 0;
          enemy.supportBurstShotTimerSeconds = 0.f;
        }
        else
        {
          enemy.supportBurstShotTimerSeconds = std::max(0.f, enemy.supportBurstShotTimerSeconds - std::max(0.f, dtSeconds));

          const ProjectileParams params = supportProjectileParams();
          if (params.speed > 0.f)
          {
            while (enemy.supportBurstShotsRemaining > 0 && enemy.supportBurstShotTimerSeconds <= 0.f)
            {
              (void)spawnSupportProjectile(
                registry,
                tilemapEntity,
                enemyEntity,
                enemyWorldPos,
                enemyRadius,
                perception.toPlayerDir,
                params,
                enemy.rangedAttackDamage,
                tileSize
              );

              enemy.supportBurstShotsRemaining -= 1;
              enemy.supportBurstShotTimerSeconds += kSupportBurstIntervalSeconds;
            }
          }
          else
          {
            enemy.supportBurstShotsRemaining = 0;
            enemy.supportBurstShotTimerSeconds = 0.f;
          }
        }
      }

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
        const bool rangedCanShoot = (perception.distWorld <= rangedAttackRangeWorld) && perception.seesPlayerNow;

        enemy.supportBurstShotsRemaining = 0;
        enemy.supportBurstShotTimerSeconds = 0.f;

        if (enemy.cls == EnemyClass::MELEE)
        {
          if (meleeInRange && canStartAttack(enemy)) enterAttacking(enemy, sprite);
          else enterMoving(enemy, sprite);
        }
        else
        {
          if (rangedCanShoot && canStartAttack(enemy))
          {
            enterAttacking(enemy, sprite);
            if (enemy.cls == EnemyClass::SUPPORT)
            {
              enemy.supportBurstShotsRemaining = kSupportBurstShots;
              enemy.supportBurstShotTimerSeconds = 0.f;
            }
          }
          else enterMoving(enemy, sprite);
        }
      }

      return true;
    }

    const bool meleeInRange = (perception.distWorld <= meleeRangeWorld);
    const bool rangedCanShoot = (perception.distWorld <= rangedAttackRangeWorld) && perception.seesPlayerNow;

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
        if (enemy.cls == EnemyClass::SUPPORT)
        {
          enemy.supportBurstShotsRemaining = kSupportBurstShots;
          enemy.supportBurstShotTimerSeconds = 0.f;
        }
        setVelocityStop(vel);
        return true;
      }
    }

    return false;
  }
}
