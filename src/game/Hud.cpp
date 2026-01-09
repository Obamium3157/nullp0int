#include "Hud.h"

#include <algorithm>
#include <cmath>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include "GameUI.h"
#include "../ecs/Components.h"

void Hud::setCrosshairStyle(const CrosshairStyle style)
{
  m_crosshair = style;
}

void Hud::setDamageVignetteStyle(const DamageVignetteStyle &style)
{
  m_vignette = style;
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

void Hud::update(const float dtSeconds,
                 ecs::Registry& registry,
                 const ecs::Entity player)
{
  const float dtSafe = std::max(0.f, dtSeconds);

  if (m_damageRemaining > 0.f)
  {
    m_damageRemaining = std::max(0.f, m_damageRemaining - dtSafe);
  }

  if (player == ecs::INVALID_ENTITY) return;
  if (!registry.isAlive(player)) return;

  const auto* hp = registry.getComponent<ecs::HealthComponent>(player);
  if (!hp) return;

  const float current = std::max(0.f, hp->current);

  if (m_lastHp < 0.f)
  {
    m_lastHp = current;
    return;
  }

  if (current + 0.0001f < m_lastHp)
  {
    m_damageRemaining = std::max(0.01f, m_vignette.durationSeconds);
  }

  m_lastHp = current;
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

  drawDamageVignette(window);
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

void Hud::drawDamageVignette(sf::RenderWindow& window) const
{
  if (m_damageRemaining <= 0.f) return;

  const float duration = std::max(0.01f, m_vignette.durationSeconds);
  const float k = clamp01(m_damageRemaining / duration);
  const float intensity = k * k;

  const int grid = std::clamp(m_vignette.grid, 5, 25);
  const sf::Vector2u ws = window.getSize();
  const auto w = static_cast<float>(ws.x);
  const auto h = static_cast<float>(ws.y);

  const float cx = w * 0.5f;
  const float cy = h * 0.5f;

  const float inner = clamp01(m_vignette.innerRadius01);
  const float expn = std::max(0.1f, m_vignette.exponent);
  const float maxA = static_cast<float>(m_vignette.maxAlpha) * intensity;

  const int cells = grid - 1;
  sf::VertexArray tris(sf::Triangles);
  tris.resize(static_cast<std::size_t>(cells * cells * 6));

  auto alphaAt = [&](const float x, const float y) -> sf::Uint8
  {
    const float nx = (cx > 0.0001f) ? ((x - cx) / cx) : 0.f;
    const float ny = (cy > 0.0001f) ? ((y - cy) / cy) : 0.f;
    const float r = std::sqrt(nx * nx + ny * ny);

    const float t = clamp01((r - inner) / std::max(0.0001f, 1.f - inner));
    const float v = std::pow(t, expn);

    const float a = std::clamp(maxA * v, 0.f, 255.f);
    return static_cast<sf::Uint8>(std::lround(a));
  };

  auto vtx = [&](const float x, const float y) -> sf::Vertex
  {
    const sf::Uint8 a = alphaAt(x, y);
    return {sf::Vector2f(x, y), sf::Color(255, 0, 0, a)};
  };

  std::size_t idx = 0;
  for (int iy = 0; iy < cells; ++iy)
  {
    const float y0 = (static_cast<float>(iy) / static_cast<float>(cells)) * h;
    const float y1 = (static_cast<float>(iy + 1) / static_cast<float>(cells)) * h;

    for (int ix = 0; ix < cells; ++ix)
    {
      const float x0 = (static_cast<float>(ix) / static_cast<float>(cells)) * w;
      const float x1 = (static_cast<float>(ix + 1) / static_cast<float>(cells)) * w;

      tris[idx++] = vtx(x0, y0);
      tris[idx++] = vtx(x1, y0);
      tris[idx++] = vtx(x1, y1);

      tris[idx++] = vtx(x0, y0);
      tris[idx++] = vtx(x1, y1);
      tris[idx++] = vtx(x0, y1);
    }
  }

  window.draw(tris);
}
