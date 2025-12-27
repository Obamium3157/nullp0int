//
// Created by obamium3157 on 27.12.2025.
//

#pragma once


#ifndef NULLP0INT_SHOTGUNWEAPON_H
#define NULLP0INT_SHOTGUNWEAPON_H

#include "Weapon.h"

namespace game::weapons
{
  class ShotgunWeapon final : public IWeapon
  {
  public:
    [[nodiscard]] WeaponId id() const override { return WeaponId::SHOTGUN; }
    [[nodiscard]] AttackType attackType() const override { return AttackType::PROJECTILE; }

    [[nodiscard]] float damage() const override { return 50.f; }

    [[nodiscard]] float fireFrameTimeSeconds() const override { return 0.09f; }
    [[nodiscard]] const std::vector<std::string>& viewIdleFrames() const override { return m_idle; }
    [[nodiscard]] const std::vector<std::string>& viewFireFrames() const override { return m_fire; }

    [[nodiscard]] std::string_view projectileTextureId() const override { return "shotgun_projectile"; }
    [[nodiscard]] float projectileSpeed() const override { return 1500.f; }
    [[nodiscard]] float projectileRadius() const override { return 5.f; }
    [[nodiscard]] float projectileVisualSizeTiles() const override { return 0.22f; }
    [[nodiscard]] float projectileSpriteScale() const override { return 1.f; }
    [[nodiscard]] float projectileHeightShift() const override { return 0.15f; }
    [[nodiscard]] float projectileLifeSeconds() const override { return 5.f; }

  private:
    std::vector<std::string> m_idle = {"shotgun_idle"};
    std::vector<std::string> m_fire = {"shotgun_fire_0", "shotgun_fire_1", "shotgun_fire_2", "shotgun_fire_3"};
  };
}

#endif //NULLP0INT_SHOTGUNWEAPON_H