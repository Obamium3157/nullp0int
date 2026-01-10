//
// Created by obamium3157 on 27.12.2025.
//

#pragma once

#ifndef NULLP0INT_PISTOLWEAPON_H
#define NULLP0INT_PISTOLWEAPON_H

#include "Weapon.h"

namespace game::weapons
{
  class PistolWeapon final : public IWeapon
  {
  public:
    [[nodiscard]] WeaponId id() const override { return WeaponId::PISTOL; }
    [[nodiscard]] AttackType attackType() const override { return AttackType::HITSCAN; }

    [[nodiscard]] float damage() const override { return 12.f; }

    [[nodiscard]] float fireFrameTimeSeconds() const override { return 0.075f; }
    [[nodiscard]] const std::vector<std::string>& viewIdleFrames() const override { return m_idle; }
    [[nodiscard]] const std::vector<std::string>& viewFireFrames() const override { return m_fire; }

    [[nodiscard]] float maxRangeTiles() const override { return 35.f; }

  private:
    std::vector<std::string> m_idle = {"pistol_idle"};
    std::vector<std::string> m_fire = {"pistol_fire_0", "pistol_fire_1", "pistol_fire_2", "pistol_fire_3", "pistol_fire_2", "pistol_fire_1", "pistol_fire_0"};
  };
}

#endif //NULLP0INT_PISTOLWEAPON_H