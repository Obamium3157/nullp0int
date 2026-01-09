//
// Created by obamium3157 on 10.01.2026.
//

#ifndef NULLP0INT_HUD_H
#define NULLP0INT_HUD_H

#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "../ecs/Entity.h"
#include "../ecs/Registry.h"

class Hud
{
public:
  struct CrosshairStyle
  {
    float armLength = 14.f;
    float thickness = 3.f;
    float gap = 7.f;
  };

  struct DamageVignetteStyle
  {
    float durationSeconds = 0.35f;
    float innerRadius01 = 0.85f;
    float exponent = 1.8f;
    sf::Uint8 maxAlpha = 140;
    int grid = 9;
  };

  void setCrosshairStyle(CrosshairStyle style);
  void setDamageVignetteStyle(const DamageVignetteStyle &style);

  void update(float dtSeconds,
              ecs::Registry& registry,
              ecs::Entity player);

  void draw(sf::RenderWindow& window,
            ecs::Registry& registry,
            ecs::Entity player,
            const sf::Font& font,
            bool fontLoaded) const;

private:
  CrosshairStyle m_crosshair;
  DamageVignetteStyle m_vignette;

  float m_lastHp = -1.f;
  float m_damageRemaining = 0.f;

  static float clamp01(float v);
  static sf::Color crosshairColorFromHealth(float current, float max);

  static void drawHealth(sf::RenderWindow& window,
                  ecs::Registry& registry,
                  ecs::Entity player,
                  const sf::Font& font,
                  bool fontLoaded) ;

  void drawCrosshair(sf::RenderWindow& window,
                     ecs::Registry& registry,
                     ecs::Entity player) const;

  void drawDamageVignette(sf::RenderWindow& window) const;
};

#endif //NULLP0INT_HUD_H