//
// Created by obamium3157 on 10.01.2026.
//

#ifndef NULLP0INT_HUD_H
#define NULLP0INT_HUD_H

#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "../ecs/Registry.h"

class Hud
{
public:
  struct CrosshairStyle
  {
    float armLength = 7.f;
    float thickness = 3.f;
    float gap = 7.f;
  };

  void setCrosshairStyle(CrosshairStyle style);

  void draw(sf::RenderWindow& window,
            ecs::Registry& registry,
            ecs::Entity player,
            const sf::Font& font,
            bool fontLoaded) const;

private:
  CrosshairStyle m_crosshair;

  static float clamp01(float v);
  static sf::Color crosshairColorFromHealth(float current, float max);

  static void drawHealth(sf::RenderWindow& window,
                         ecs::Registry& registry,
                         ecs::Entity player,
                         const sf::Font& font,
                         bool fontLoaded);

  void drawCrosshair(sf::RenderWindow& window,
                     ecs::Registry& registry,
                     ecs::Entity player) const;
};


#endif //NULLP0INT_HUD_H