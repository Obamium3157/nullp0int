//
// Created by obamium3157 on 27.12.2025.
//

#pragma once

#ifndef NULLP0INT_WEAPON_H
#define NULLP0INT_WEAPON_H

#include <string>
#include <vector>

namespace game::weapons
{
  enum class WeaponId
  {
    PISTOL,
    SHOTGUN
  };

  enum class AttackType
  {
    HITSCAN,
    PROJECTILE
  };

  class IWeapon
  {
  public:
    virtual ~IWeapon() = default;

    [[nodiscard]] virtual WeaponId id() const = 0;
    [[nodiscard]] virtual AttackType attackType() const = 0;

    [[nodiscard]] virtual float damage() const = 0;

    [[nodiscard]] virtual float fireFrameTimeSeconds() const = 0;
    [[nodiscard]] virtual const std::vector<std::string>& viewIdleFrames() const = 0;
    [[nodiscard]] virtual const std::vector<std::string>& viewFireFrames() const = 0;

    [[nodiscard]] float fireDurationSeconds() const
    {
      const auto& frames = viewFireFrames();
      if (frames.empty()) return 0.f;
      return static_cast<float>(frames.size()) * fireFrameTimeSeconds();
    }

    [[nodiscard]] virtual float maxRangeTiles() const { return 30.f; }

    [[nodiscard]] virtual std::string_view projectileTextureId() const { return {}; }
    [[nodiscard]] virtual float projectileSpeed() const { return 0.f; }
    [[nodiscard]] virtual float projectileRadius() const { return 0.f; }
    [[nodiscard]] virtual float projectileVisualSizeTiles() const { return 0.25f; }
    [[nodiscard]] virtual float projectileSpriteScale() const { return 1.f; }
    [[nodiscard]] virtual float projectileHeightShift() const { return 0.12f; }
    [[nodiscard]] virtual float projectileLifeSeconds() const { return 2.0f; }
  };
}

#endif //NULLP0INT_WEAPON_H