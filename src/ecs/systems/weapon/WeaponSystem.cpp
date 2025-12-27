//
// Created by obamium3157 on 27.12.2025.
//

#include "WeaponSystem.h"

#include <algorithm>
#include <cmath>
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
    ecs::Registry &    registry,
    const sf::Vector2f playerPos,
    const sf::Vector2f dir,
    const float        maxDist
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

  const bool fireNow = sf::Mouse::isButtonPressed(sf::Mouse::Left);

  const auto* pos = registry.getComponent<PositionComponent>(playerEntity);
  const auto* rot = registry.getComponent<RotationComponent>(playerEntity);
  const auto* rad = registry.getComponent<RadiusComponent>(playerEntity);
  if (!pos || !rot) return;

  if (fireNow && activeSlot.cooldownRemaining <= 0.f)
  {
    const float ang = radiansFromDegrees(rot->angle);
    const sf::Vector2f dir{ std::cos(ang), std::sin(ang) };

    if (activeSlot.weapon->attackType() == game::weapons::AttackType::HITSCAN)
    {
      const float wallDist = getAimWallDistance(registry, playerEntity);
      const float maxRange = activeSlot.weapon->maxRangeTiles() * config.tile_size;

      float maxDist = maxRange;
      if (wallDist > 0.f) maxDist = std::min(maxDist, wallDist);

      if (maxDist > 1.f)
      {
        const Entity hitEnemy = hitscanPickEnemy(registry, pos->position, dir, maxDist
        );
        if (hitEnemy != INVALID_ENTITY)
        {
          applyDamageOrKill(registry, hitEnemy, activeSlot.weapon->damage());
        }
      }
    }
    else
    {
      const float ownerRadius = rad ? rad->radius : config.player_radius;
      (void)spawnProjectileFromWeapon(registry, config, tilemapEntity, playerEntity, pos->position, ownerRadius, dir, *activeSlot.weapon);
    }

    startFireAnimation(activeSlot);
  }

  inputState->prevFire = fireNow;
}