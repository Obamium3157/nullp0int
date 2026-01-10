//
// Created by obamium3157 on 13.11.2025.
//

#pragma once

#ifndef NULLP0INT_COMPONENTS_H
#define NULLP0INT_COMPONENTS_H

#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#include "../constants.h"
#include "../game/weapons/PistolWeapon.h"
#include "systems/render/Animation.h"

namespace ecs
{
  struct PositionComponent
  {
    sf::Vector2f position{};
  };

  struct VelocityComponent
  {
    sf::Vector2f velocity{};
    float velocityMultiplier = 1.f;
  };
  struct RotationVelocityComponent
  {
    float rotationVelocity = 0.f;
  };

  struct SpeedComponent
  {
    float speed = 0.f;
  };
  struct RotationSpeedComponent
  {
    float rotationSpeed = 0.f;
  };

  struct RotationComponent
  {
    float angle = 0.f;
  };

  struct RadiusComponent
  {
    float radius = 0.f;
  };

  struct HealthComponent
  {
    float current = 100.f;
    float max = 100.f;
  };

  struct PlayerTag{};
  struct PlayerInput
  {
    float moveSpeed = 150.f;
  };

  enum class EnemyClass
  {
    MELEE,
    RANGE,
    SUPPORT,
  };

  enum class EnemyState
  {
    PASSIVE,
    MOVING,
    ATTACKING,
  };

  struct EnemyComponent
  {
    EnemyClass cls = EnemyClass::MELEE;
    std::string textureId;
    float spriteScale = 1.f;
    float heightShift = 0.27f;

    bool hasSeenPlayer = false;
    float visionRangeTiles = 40.f;
    float fovDegrees = 360.f;

    EnemyState state = EnemyState::PASSIVE;

    float meleeAttackRangeTiles = 1.15f;
    float meleeAttackDamage = 10.f;

    float rangedPreferredRangeTiles = 10.f;
    float rangedRangeToleranceTiles = 2.f;
    float rangedAttackRangeTiles = 18.f;
    float rangedAttackDamage = 6.f;

    float attackCooldownSeconds = 0.f;
    float cooldownRemainingSeconds = 0.f;

    bool attackDamageApplied = false;
    std::size_t attackApplyFrame = 0;

    std::vector<std::string> idleFrames;
    std::vector<std::string> walkFrames;
    std::vector<std::string> walkFramesLeft;
    std::vector<std::string> walkFramesRight;
    std::vector<std::string> walkFramesBack;
    std::vector<std::string> attackFrames;
    float walkFrameTime = 0.09f;
    float attackFrameTime = 0.07f;

    int supportBurstShotsRemaining = 0;
    float supportBurstShotTimerSeconds = 0.f;

    bool rangedDodgeActive = false;
    float rangedDodgeTimeRemainingSeconds = 0.f;
    int rangedDodgeDir = 0;
    sf::Vector2f rangedDodgeWorldDir{0.f, 0.f};
    std::uint32_t rngState = 0;
  };
  struct EnemyTag{};


  struct TilemapTag{};


  struct TileAppearance
  {
    std::string singleTextureId{};
    std::vector<std::string> frames;
    float frameTime = 0.1f;
    float lastUpdateTime = 0.f;

    [[nodiscard]] bool isAnimated() const { return !frames.empty(); }
    [[nodiscard]] const std::string& currentTextureId(const float currentTime) const
    {
      if (!isAnimated()) return singleTextureId;
      if (frameTime <= 0.f) return frames.front();

      const float localTime = currentTime - lastUpdateTime;
      const size_t idx = static_cast<size_t>(std::floor(localTime / frameTime)) % frames.size();
      return frames[idx];
    }
  };

  struct TilemapComponent
  {
    unsigned width = 0;
    unsigned height = 0;
    float tileSize = 64.f;
    std::vector<std::string> tiles;
    std::string floorTextureId;

    std::unordered_map<char, TileAppearance> tileAppearanceMap;

    [[nodiscard]] bool isWall(const int tx, const int ty) const
    {
      if (tx < 0 || ty < 0 || tx >= static_cast<int>(width) || ty >= static_cast<int>(height))
        return false;

      const char c = tiles[ty][tx];

      return c != FLOOR_MARKER
        && c != SPAWN_MARKER
        && c != MELEE_ENEMY_SPAWN_MARKER
        && c != RANGE_ENEMY_SPAWN_MARKER
        && c != SUPPORT_ENEMY_SPAWN_MARKER;
    }

    [[nodiscard]] sf::Vector2f getSpawnPosition() const
    {
      for (unsigned ty = 0; ty < height; ++ty)
      {
        for (unsigned tx = 0; tx < width; ++tx)
        {
          if (tiles[ty][tx] == SPAWN_MARKER)
          {
            return sf::Vector2f{
              static_cast<float>(tx) * tileSize + tileSize / 2.f,
              static_cast<float>(ty) * tileSize + tileSize / 2.f
            };
          }
        }
      }

      return sf::Vector2f{
        static_cast<float>(width) * tileSize / 2.f,
        static_cast<float>(height) * tileSize / 2.f
      };
    }

    [[nodiscard]] sf::Vector2i worldToTile(const sf::Vector2f worldPos) const
    {
      return {static_cast<int>(worldPos.x / tileSize), static_cast<int>(worldPos.y / tileSize)};
    }
  };

  struct RayHit
  {
    sf::Vector2f hitPointWorld;
    float distance = 0.f;
    int tileX = -1;
    int tileY = -1;
    bool vertical = false;
    float rayAngle = 0.f;
  };

  struct RayCastResultComponent
  {
    std::vector<RayHit> hits;
  };

  struct SpriteComponent
  {
    std::string textureId;
    std::vector<sf::IntRect> frames;
    std::vector<std::string> textureFrames;
    float frameTime = 0.1f;
    bool playing = true;
    bool loop = true;
    std::size_t currentFrame = 0;
    float frameAccumulator = 0.f;
  };

  struct ProjectileTag {};

  struct ProjectileComponent
  {
    Entity owner = INVALID_ENTITY;

    sf::Vector2f positionPrev{};
    sf::Vector2f direction{1.f, 0.f};

    float speed = 2500.f;
    float damage = 50.f;

    float radius = 5.f;
    float lifeSeconds = 2.0f;
    float livedSeconds = 0.f;

    std::string textureId{};
    float spriteScale = 1.f;
    float heightShift = 0.12f;
    float visualSizeTiles = 0.25f;

    float ignoreOwnerSeconds = 0.04f;

    bool parried = false;
  };

  struct InvulnerabilityComponent
  {
    float remainingSeconds = 0.f;
  };

  struct ParryComponent
  {
    float cooldownRemainingSeconds = 0.f;
    bool parrying = false;
    std::size_t animFrame = 0;
    float animAccumulator = 0.f;

    float crosshairFlashRemainingSeconds = 0.f;
  };

  struct HitMarkerComponent
  {
    float remainingSeconds = 0.f;
  };

  struct PlayerWeaponInputState
  {
    bool prevFire = false;
    bool prevNum2 = false;
    bool prevNum3 = false;
    bool prevParry = false;
  };

  struct WeaponSlotRuntime
  {
    std::shared_ptr<game::weapons::IWeapon> weapon{};
    float cooldownRemaining = 0.f;
    bool firing = false;

    std::size_t animFrame = 0;
    float animAccumulator = 0.f;
  };

  struct WeaponInventoryComponent
  {
    std::vector<WeaponSlotRuntime> slots;
    int activeIndex = 0;
  };

}

#endif //NULLP0INT_COMPONENTS_H
