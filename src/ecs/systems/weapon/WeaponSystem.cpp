//
// Created by obamium3157 on 27.12.2025.
//

#include "WeaponSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include "../../Components.h"
#include "../../../game/weapons/Weapon.h"
#include "../../../math/mathUtils.h"
#include "../collision/CollisionSystem.h"

namespace
{
  [[nodiscard]] bool keyPressedAny(const sf::Keyboard::Key a, const sf::Keyboard::Key b)
  {
    return sf::Keyboard::isKeyPressed(a) || sf::Keyboard::isKeyPressed(b);
  }

  [[nodiscard]] int findWeaponSlotIndex(const ecs::WeaponInventoryComponent& inv, const game::weapons::WeaponId id)
  {
    for (int i = 0; i < static_cast<int>(inv.slots.size()); ++i)
    {
      if (inv.slots[static_cast<std::size_t>(i)].weapon && inv.slots[static_cast<std::size_t>(i)].weapon->id() == id)
        return i;
    }
    return -1;
  }

  void ensureHitMarker(ecs::Registry& registry, const ecs::Entity playerEntity)
  {
    if (!registry.hasComponent<ecs::HitMarkerComponent>(playerEntity))
    {
      registry.addComponent<ecs::HitMarkerComponent>(playerEntity, ecs::HitMarkerComponent{HITMARKER_DURATION_SECONDS});
      return;
    }

    if (auto* hm = registry.getComponent<ecs::HitMarkerComponent>(playerEntity))
    {
      hm->remainingSeconds = std::max(hm->remainingSeconds, HITMARKER_DURATION_SECONDS);
    }
  }

  void applyDamageOrKill(ecs::Registry& registry, const ecs::Entity target, const float dmg)
  {
    if (target == ecs::INVALID_ENTITY) return;

    if (auto* hp = registry.getComponent<ecs::HealthComponent>(target))
    {
      hp->current -= dmg;
      if (hp->current <= 0.f)
      {
        registry.destroyEntity(target);
      }
    }
  }

  ecs::Entity hitscanPickEnemy(
    ecs::Registry& registry,
    const sf::Vector2f playerPos,
    const sf::Vector2f dir,
    const float maxDist
  )
  {
    ecs::Entity best = ecs::INVALID_ENTITY;
    float bestT = maxDist + 1.f;

    for (const auto& e : registry.entities())
    {
      if (!registry.hasComponent<ecs::EnemyTag>(e)) continue;

      const auto* ep = registry.getComponent<ecs::PositionComponent>(e);
      const auto* er = registry.getComponent<ecs::RadiusComponent>(e);
      if (!ep || !er) continue;

      const sf::Vector2f to{ ep->position.x - playerPos.x, ep->position.y - playerPos.y };
      const float t = to.x * dir.x + to.y * dir.y;
      if (t <= 0.f || t > maxDist) continue;

      const sf::Vector2f closest{ playerPos.x + dir.x * t, playerPos.y + dir.y * t };
      const float dx = ep->position.x - closest.x;
      const float dy = ep->position.y - closest.y;
      if (const float dist2 = dx * dx + dy * dy; dist2 > er->radius * er->radius) continue;

      if (t < bestT)
      {
        bestT = t;
        best = e;
      }
    }

    return best;
  }

  [[nodiscard]] float getAimWallDistance(ecs::Registry& registry, const ecs::Entity player)
  {
    const auto* rays = registry.getComponent<ecs::RayCastResultComponent>(player);
    if (!rays || rays->hits.empty()) return 0.f;

    const std::size_t mid = rays->hits.size() / 2u;
    const auto& hit = rays->hits[mid];
    return (hit.distance > 0.f && std::isfinite(hit.distance)) ? hit.distance : 0.f;
  }

  void startFireAnimation(ecs::WeaponSlotRuntime& slot)
  {
    if (!slot.weapon) return;

    slot.cooldownRemaining = std::max(0.f, slot.weapon->fireDurationSeconds());
    slot.firing = slot.cooldownRemaining > 0.f;

    slot.animFrame = 0;
    slot.animAccumulator = 0.f;
  }

  void updateWeaponTimers(ecs::WeaponInventoryComponent& inv, const float dt)
  {
    for (auto& slot : inv.slots)
    {
      if (!slot.weapon) continue;

      if (slot.cooldownRemaining > 0.f)
      {
        slot.cooldownRemaining = std::max(0.f, slot.cooldownRemaining - dt);

        const float frameTime = slot.weapon->fireFrameTimeSeconds();

        if (const auto& frames = slot.weapon->viewFireFrames(); frameTime > 0.f && !frames.empty())
        {
          slot.animAccumulator += dt;
          while (slot.animAccumulator >= frameTime && slot.animFrame + 1u < frames.size())
          {
            slot.animAccumulator -= frameTime;
            slot.animFrame++;
          }
        }

        slot.firing = slot.cooldownRemaining > 0.f;
        if (!slot.firing)
        {
          slot.animFrame = 0;
          slot.animAccumulator = 0.f;
        }
      }
      else
      {
        slot.firing = false;
        slot.animFrame = 0;
        slot.animAccumulator = 0.f;
      }
    }
  }

  void updateInvulnerability(ecs::Registry& registry, const ecs::Entity player, const float dt)
  {
    if (auto* inv = registry.getComponent<ecs::InvulnerabilityComponent>(player))
    {
      inv->remainingSeconds = std::max(0.f, inv->remainingSeconds - std::max(0.f, dt));
      if (inv->remainingSeconds <= 0.f)
      {
        registry.removeComponent<ecs::InvulnerabilityComponent>(player);
      }
    }
  }

  void updateParryAnimation(ecs::ParryComponent& parry, const float dt)
  {
    if (parry.cooldownRemainingSeconds > 0.f)
    {
      parry.cooldownRemainingSeconds = std::max(0.f, parry.cooldownRemainingSeconds - std::max(0.f, dt));
    }

    if (parry.crosshairFlashRemainingSeconds > 0.f)
    {
      parry.crosshairFlashRemainingSeconds = std::max(0.f, parry.crosshairFlashRemainingSeconds - std::max(0.f, dt));
    }

    if (!parry.parrying) return;

    constexpr float frameTime = std::max(0.f, PARRY_ANIM_FRAME_TIME_SECONDS);

    parry.animAccumulator += dt;
    while (parry.animAccumulator >= frameTime)
    {
      parry.animAccumulator -= frameTime;

      if (parry.animFrame + 1u < 3u)
      {
        parry.animFrame++;
      }
      else
      {
        parry.parrying = false;
        parry.animFrame = 0;
        parry.animAccumulator = 0.f;
        break;
      }
    }
  }

  [[nodiscard]] float length(const sf::Vector2f v)
  {
    return std::hypot(v.x, v.y);
  }

  [[nodiscard]] sf::Vector2f normalizeSafe(const sf::Vector2f v)
  {
    const float len = length(v);
    if (!std::isfinite(len) || len <= 1e-6f) return {0.f, 0.f};
    return {v.x / len, v.y / len};
  }

  [[nodiscard]] float angleBetweenRadians(const sf::Vector2f aNorm, const sf::Vector2f bNorm)
  {
    const float dot = std::clamp(aNorm.x * bNorm.x + aNorm.y * bNorm.y, -1.f, 1.f);
    return std::acos(dot);
  }

  ecs::Entity pickParryProjectile(
    ecs::Registry& registry,
    const sf::Vector2f playerPos,
    const sf::Vector2f lookDirNorm,
    const float rangeWorld,
    const float fovRad
  )
  {
    ecs::Entity best = ecs::INVALID_ENTITY;
    float bestDist = std::numeric_limits<float>::infinity();

    for (const auto& e : registry.entities())
    {
      if (!registry.hasComponent<ecs::ProjectileTag>(e)) continue;

      const auto* pp = registry.getComponent<ecs::PositionComponent>(e);
      const auto* prj = registry.getComponent<ecs::ProjectileComponent>(e);
      if (!pp || !prj) continue;

      const sf::Vector2f to{ pp->position.x - playerPos.x, pp->position.y - playerPos.y };
      const float dist = length(to);
      if (!std::isfinite(dist) || dist > rangeWorld) continue;

      const sf::Vector2f toDir = normalizeSafe(to);
      if (toDir.x == 0.f && toDir.y == 0.f) continue;

      if (angleBetweenRadians(lookDirNorm, toDir) > fovRad) continue;

      if (dist < bestDist)
      {
        bestDist = dist;
        best = e;
      }
    }

    return best;
  }

  void applyParryToProjectile(
    ecs::Registry& registry,
    const ecs::Entity projectile,
    const ecs::Entity playerEntity,
    const sf::Vector2f lookDirNorm
  )
  {
    auto* prj = registry.getComponent<ecs::ProjectileComponent>(projectile);
    if (!prj) return;

    const bool wasEnemy = (prj->owner != ecs::INVALID_ENTITY) && registry.hasComponent<ecs::EnemyTag>(prj->owner);

    prj->owner = playerEntity;
    prj->damage = PARRY_PROJECTILE_DAMAGE;

    if (!prj->parried)
    {
      prj->speed *= PARRY_PROJECTILE_SPEED_MULTIPLIER;
    }

    prj->direction = lookDirNorm;
    prj->textureId = "parried_projectile";
    prj->parried = true;

    if (wasEnemy)
    {
      if (auto* hp = registry.getComponent<ecs::HealthComponent>(playerEntity))
      {
        hp->current = std::min(hp->max, hp->current + PARRY_HEAL_ON_ENEMY_PROJECTILE);
      }
    }
  }

  ecs::Entity spawnProjectileFromWeapon(
    ecs::Registry& registry,
    const Configuration& config,
    const ecs::Entity tilemapEntity,
    const ecs::Entity owner,
    const sf::Vector2f ownerPos,
    const float ownerRadius,
    const sf::Vector2f dir,
    const game::weapons::IWeapon& weapon
  )
  {
    const float pr = std::max(0.5f, weapon.projectileRadius());
    const float spawnBaseOffset = ownerRadius + pr + 6.f;

    sf::Vector2f spawnPos{ ownerPos.x + dir.x * spawnBaseOffset, ownerPos.y + dir.y * spawnBaseOffset };

    for (int i = 0; i < 8; ++i)
    {
      if (!ecs::CollisionSystem::checkWallCollision(registry, spawnPos, pr, tilemapEntity)) break;
      spawnPos.x += dir.x * (config.tile_size * 0.20f);
      spawnPos.y += dir.y * (config.tile_size * 0.20f);
    }

    if (ecs::CollisionSystem::checkWallCollision(registry, spawnPos, pr, tilemapEntity))
    {
      return ecs::INVALID_ENTITY;
    }

    const ecs::Entity proj = registry.createEntity();
    registry.addComponent<ecs::PositionComponent>(proj, ecs::PositionComponent{spawnPos});
    registry.addComponent<ecs::ProjectileTag>(proj, ecs::ProjectileTag{});

    ecs::ProjectileComponent pc;
    pc.owner = owner;
    pc.positionPrev = spawnPos;
    pc.direction = dir;
    pc.speed = weapon.projectileSpeed();
    pc.damage = weapon.damage();
    pc.radius = pr;
    pc.lifeSeconds = weapon.projectileLifeSeconds();
    pc.livedSeconds = 0.f;
    pc.textureId = std::string(weapon.projectileTextureId());
    pc.spriteScale = weapon.projectileSpriteScale();
    pc.heightShift = weapon.projectileHeightShift();
    pc.visualSizeTiles = weapon.projectileVisualSizeTiles();
    pc.ignoreOwnerSeconds = 0.04f;
    pc.parried = false;

    registry.addComponent<ecs::ProjectileComponent>(proj, pc);
    return proj;
  }
}

void ecs::WeaponSystem::update(Registry& registry, const Configuration& config, const Entity tilemapEntity, const Entity playerEntity, const float dtSeconds)
{
  if (playerEntity == INVALID_ENTITY) return;
  if (!registry.isAlive(playerEntity)) return;

  auto* inv = registry.getComponent<WeaponInventoryComponent>(playerEntity);
  if (!inv) return;

  auto* inputState = registry.getComponent<PlayerWeaponInputState>(playerEntity);
  if (!inputState)
  {
    registry.addComponent<PlayerWeaponInputState>(playerEntity, PlayerWeaponInputState{});
    inputState = registry.getComponent<PlayerWeaponInputState>(playerEntity);
    if (!inputState) return;
  }

  auto* parry = registry.getComponent<ParryComponent>(playerEntity);
  if (!parry)
  {
    registry.addComponent<ParryComponent>(playerEntity, ParryComponent{});
    parry = registry.getComponent<ParryComponent>(playerEntity);
    if (!parry) return;
  }

  updateInvulnerability(registry, playerEntity, dtSeconds);
  updateParryAnimation(*parry, dtSeconds);
  updateWeaponTimers(*inv, dtSeconds);

  const bool num2Now = keyPressedAny(sf::Keyboard::Num2, sf::Keyboard::Numpad2);
  const bool num3Now = keyPressedAny(sf::Keyboard::Num3, sf::Keyboard::Numpad3);

  if (num2Now && !inputState->prevNum2)
  {
    if (const int idx = findWeaponSlotIndex(*inv, game::weapons::WeaponId::PISTOL); idx >= 0) inv->activeIndex = idx;
  }
  if (num3Now && !inputState->prevNum3)
  {
    if (const int idx = findWeaponSlotIndex(*inv, game::weapons::WeaponId::SHOTGUN); idx >= 0) inv->activeIndex = idx;
  }

  inputState->prevNum2 = num2Now;
  inputState->prevNum3 = num3Now;

  if (inv->slots.empty()) return;

  inv->activeIndex = std::clamp(inv->activeIndex, 0, static_cast<int>(inv->slots.size()) - 1);
  auto& activeSlot = inv->slots[static_cast<std::size_t>(inv->activeIndex)];

  if (!activeSlot.weapon) return;

  const auto* pos = registry.getComponent<PositionComponent>(playerEntity);
  const auto* rot = registry.getComponent<RotationComponent>(playerEntity);
  const auto* rad = registry.getComponent<RadiusComponent>(playerEntity);
  if (!pos || !rot) return;

  const float angRad = radiansFromDegrees(rot->angle);
  const sf::Vector2f lookDirNorm{ std::cos(angRad), std::sin(angRad) };

  const bool parryNow = sf::Keyboard::isKeyPressed(sf::Keyboard::F);
  const bool fireNow = sf::Mouse::isButtonPressed(sf::Mouse::Left);

  const bool canStartParry =
    parryNow &&
    !inputState->prevParry &&
    !parry->parrying &&
    parry->cooldownRemainingSeconds <= 0.f &&
    !activeSlot.firing &&
    activeSlot.cooldownRemaining <= 0.f;

  if (canStartParry)
  {
    parry->parrying = true;
    parry->animFrame = 0;
    parry->animAccumulator = 0.f;
    parry->cooldownRemainingSeconds = std::max(0.f, PARRY_COOLDOWN_SECONDS);

    if (!registry.hasComponent<InvulnerabilityComponent>(playerEntity))
    {
      registry.addComponent<InvulnerabilityComponent>(playerEntity, InvulnerabilityComponent{PARRY_INVULNERABILITY_SECONDS});
    }
    else if (auto* invul = registry.getComponent<InvulnerabilityComponent>(playerEntity))
    {
      invul->remainingSeconds = std::max(invul->remainingSeconds, PARRY_INVULNERABILITY_SECONDS);
    }

    const float rangeWorld = std::max(0.f, PARRY_RANGE_TILES) * config.tile_size;
    const float fovRad = radiansFromDegrees(std::max(0.f, PARRY_FOV_DEGREES));

    const Entity targetProjectile = pickParryProjectile(registry, pos->position, lookDirNorm, rangeWorld, fovRad);
    if (targetProjectile != INVALID_ENTITY)
    {
      applyParryToProjectile(registry, targetProjectile, playerEntity, lookDirNorm);
      parry->crosshairFlashRemainingSeconds = std::max(parry->crosshairFlashRemainingSeconds, std::max(0.f, PARRY_CROSSHAIR_FLASH_SECONDS));
    }
  }

  inputState->prevParry = parryNow;

  if (!parry->parrying && fireNow && activeSlot.cooldownRemaining <= 0.f)
  {
    const sf::Vector2f aimDir = lookDirNorm;

    if (activeSlot.weapon->attackType() == game::weapons::AttackType::HITSCAN)
    {
      const float wallDist = getAimWallDistance(registry, playerEntity);
      const float maxRange = activeSlot.weapon->maxRangeTiles() * config.tile_size;

      float maxDist = maxRange;
      if (wallDist > 0.f) maxDist = std::min(maxDist, wallDist);

      if (maxDist > 1.f)
      {
        const Entity hitEnemy = hitscanPickEnemy(registry, pos->position, aimDir, maxDist);
        if (hitEnemy != INVALID_ENTITY)
        {
          applyDamageOrKill(registry, hitEnemy, activeSlot.weapon->damage());
          ensureHitMarker(registry, playerEntity);
        }
      }
    }
    else
    {
      const float ownerRadius = rad ? rad->radius : config.player_radius;
      (void)spawnProjectileFromWeapon(registry, config, tilemapEntity, playerEntity, pos->position, ownerRadius, aimDir, *activeSlot.weapon);
    }

    startFireAnimation(activeSlot);
  }

  inputState->prevFire = fireNow;
}