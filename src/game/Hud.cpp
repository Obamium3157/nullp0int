//
// Created by obamium3157 on 10.01.2026.
//

#include "Hud.h"

#include <algorithm>
#include <cmath>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

#include "GameUI.h"
#include "../ecs/Components.h"

void Hud::setCrosshairStyle(const CrosshairStyle style)
{
  m_crosshair = style;
}

float Hud::clamp01(const float v)
{
  return std::clamp(v, 0.f, 1.f);
}

sf::Color Hud::crosshairColorFromHealth(const float current, const float max)
{
  const float safeMax = (max > 0.0001f) ? max : 1.f;
  const float damage = clamp01(1.f - (current / safeMax));

  const auto gb = static_cast<sf::Uint8>(std::lround(255.f * (1.f - damage)));
  return {255, gb, gb, 255};
}

void Hud::draw(sf::RenderWindow& window,
              ecs::Registry& registry,
              const ecs::Entity player,
              const sf::Font& font,
              const bool fontLoaded) const
{
  if (player == ecs::INVALID_ENTITY) return;
  if (!registry.isAlive(player)) return;

  const sf::View prevView = window.getView();
  window.setView(window.getDefaultView());

  drawCrosshair(window, registry, player);
  drawHealth(window, registry, player, font, fontLoaded);

  window.setView(prevView);
}

void Hud::drawHealth(sf::RenderWindow& window,
                    ecs::Registry& registry,
                    const ecs::Entity player,
                    const sf::Font& font,
                    const bool fontLoaded)
{
  if (!fontLoaded) return;

  const auto* hp = registry.getComponent<ecs::HealthComponent>(player);
  if (!hp) return;

  sf::Text text;
  text.setFont(font);
  text.setCharacterSize(28);
  text.setFillColor(sf::Color::White);

  const int cur = static_cast<int>(std::round(hp->current));
  const int mx = static_cast<int>(std::round(hp->max));
  text.setString(toSfStringUtf8("HP: " + std::to_string(cur) + "/" + std::to_string(mx)));

  const auto bounds = text.getLocalBounds();
  constexpr float marginX = 18.f;
  constexpr float marginY = 14.f;
  const float y = static_cast<float>(window.getSize().y) - (bounds.top + bounds.height) - marginY;
  text.setPosition(marginX, y);
  window.draw(text);
}

void Hud::drawCrosshair(sf::RenderWindow& window,
                        ecs::Registry& registry,
                        const ecs::Entity player) const
{
  const auto* hp = registry.getComponent<ecs::HealthComponent>(player);
  if (!hp) return;

  const sf::Vector2u ws = window.getSize();
  const sf::Vector2f center(static_cast<float>(ws.x) * 0.5f, static_cast<float>(ws.y) * 0.5f);

  const float len = std::max(1.f, m_crosshair.armLength);
  const float thick = std::max(1.f, m_crosshair.thickness);
  const float gap = std::max(0.f, m_crosshair.gap);

  const sf::Color color = crosshairColorFromHealth(hp->current, hp->max);

  const sf::Vector2f size(len, thick);
  const sf::Vector2f origin(size.x * 0.5f, size.y * 0.5f);
  const float offset = (gap * 0.5f) + (len * 0.5f);

  sf::RectangleShape bar;
  bar.setSize(size);
  bar.setOrigin(origin);
  bar.setFillColor(color);

  bar.setRotation(0.f);
  bar.setPosition(center.x + offset, center.y);
  window.draw(bar);

  bar.setPosition(center.x - offset, center.y);
  window.draw(bar);

  bar.setRotation(90.f);
  bar.setPosition(center.x, center.y + offset);
  window.draw(bar);

  bar.setPosition(center.x, center.y - offset);
  window.draw(bar);
}
