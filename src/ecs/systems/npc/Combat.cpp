//
// Created by obamium3157 on 09.01.2026.
//

#include "Combat.h"

#include <algorithm>

#include "../collision/CollisionSystem.h"

#include "Movement.h"
#include "PathfindingAnimation.h"
#include "PathfindingTypes.h"

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

  [[nodiscard]] static std::uint32_t xorshift32(std::uint32_t& s)
  {
    if (s == 0u) s = 0xA3C59AC3u;
    s ^= s << 13;
    s ^= s >> 17;
    s ^= s << 5;
    return s;
  }

  [[nodiscard]] static float rand01(std::uint32_t& s)
  {
    const std::uint32_t r = xorshift32(s);
    return static_cast<float>(r & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
  }

  enum class DodgeDir
  {
    Left = 0,
    Right = 1,
    Back = 2,
  };

  [[nodiscard]] static DodgeDir pickDodgeDir(EnemyComponent& enemy)
  {
    const int v = std::clamp(static_cast<int>(rand01(enemy.rngState) * 3.f), 0, 2);
    return static_cast<DodgeDir>(v);
  }

  [[nodiscard]] static float pickDodgeDurationSeconds(EnemyComponent& enemy)
  {
    constexpr float kMin = 0.2f;
    constexpr float kMax = 1.5f;
    return kMin + rand01(enemy.rngState) * (kMax - kMin);
  }

  static void beginRangedDodge(
    EnemyComponent& enemy,
    SpriteComponent& sprite,
    const PerceptionResult& perception
  )
  {
    const DodgeDir d = pickDodgeDir(enemy);
    enemy.rangedDodgeActive = true;
    enemy.rangedDodgeTimeRemainingSeconds = pickDodgeDurationSeconds(enemy);
    enemy.rangedDodgeDir = static_cast<int>(d);

    sf::Vector2f dir{0.f, 0.f};
    const sf::Vector2f toPlayer = perception.toPlayerDir;

    const std::vector<std::string>* anim = &enemy.walkFrames;

    switch (d)
    {
      case DodgeDir::Left:
        dir = perpendicularStrafeDir(toPlayer, false);
        if (!enemy.walkFramesLeft.empty()) anim = &enemy.walkFramesLeft;
        break;
      case DodgeDir::Right:
        dir = perpendicularStrafeDir(toPlayer, true);
        if (!enemy.walkFramesRight.empty()) anim = &enemy.walkFramesRight;
        break;
      case DodgeDir::Back:
        dir = sf::Vector2f{-toPlayer.x, -toPlayer.y};
        if (!enemy.walkFramesBack.empty()) anim = &enemy.walkFramesBack;
        break;
    }

    enemy.rangedDodgeWorldDir = normalizedOrZero(dir);

    enemy.state = EnemyState::MOVING;
    if (anim && !anim->empty())
    {
      setAnimation(sprite, *anim, enemy.walkFrameTime, true, true);
    }
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

          if (const ProjectileParams params = supportProjectileParams(); params.speed > 0.f)
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

      if (enemy.cls != EnemyClass::SUPPORT)
      {
        if (!enemy.attackDamageApplied)
        {
          const std::size_t applyFrame = enemy.attackApplyFrame;
          if ((!sprite.playing) || (sprite.currentFrame >= applyFrame))
          {
            applyDamage(enemy, playerHealth, perception, meleeRangeWorld, rangedAttackRangeWorld);
            enemy.attackDamageApplied = true;
          }
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
          else
          {
            if (rangedCanShoot)
            {
              beginRangedDodge(enemy, sprite, perception);
            }
            else
            {
              enterMoving(enemy, sprite);
            }
          }
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
