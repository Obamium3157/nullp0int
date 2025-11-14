//
// Created by obamium3157 on 14.11.2025.
//

#include "RenderSystem.h"

#include <algorithm>
#include <cmath>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include "../../Components.h"
#include "../../Registry.h"
#include "../../../constants.h"
#include "../../../math/mathUtils.h"

void ecs::RenderSystem::render(Registry &registry, sf::RenderWindow &window)
{
  const sf::Color ceilingColor(4, 2, 115);
  const sf::Color floorColor(50, 50, 50);

  sf::RectangleShape ceiling(sf::Vector2f(static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT) / 2.f));
  ceiling.setPosition(0.f, 0.f);
  ceiling.setFillColor(ceilingColor);
  window.draw(ceiling);

  sf::RectangleShape floor(sf::Vector2f(static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT) / 2.f));
  floor.setPosition(0.f, static_cast<float>(SCREEN_HEIGHT) / 2.f);
  floor.setFillColor(floorColor);
  window.draw(floor);

  for (const auto &e : registry.entities())
  {
    if (!registry.hasComponent<PositionComponent>(e)) continue;
    if (!registry.hasComponent<RotationComponent>(e)) continue;

    auto* pos = registry.getComponent<PositionComponent>(e);
    auto* rot = registry.getComponent<RotationComponent>(e);
    if (!pos || !rot) continue;

    if (!registry.hasComponent<RayCastResultComponent>(e)) continue;
    auto* rr = registry.getComponent<RayCastResultComponent>(e);
    if (!rr || rr->hits.empty()) continue;

    constexpr float halfScreenWidth = static_cast<float>(SCREEN_WIDTH) / 2.f;
    const float     screenDist      = halfScreenWidth / std::tan(HALF_FOV);
    constexpr float columnWidth     = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(AMOUNT_OF_RAYS);
    constexpr float tileSize        = TILE_SCALE;

    const float ang = radiansFromDegrees(rot->angle);
    float rayAngle = ang - HALF_FOV + RAY_ANGLE_OFFSET;

    for (unsigned i = 0; i < rr->hits.size() && i < AMOUNT_OF_RAYS; ++i)
    {
      const RayHit &hit = rr->hits[i];

      if (hit.distance <= 0.f || !std::isfinite(hit.distance))
      {
        rayAngle += DELTA_ANGLE;
        continue;
      }

      const float correctedDepth = hit.distance * std::cos(ang - rayAngle);
      const float projH = screenDist * tileSize / (correctedDepth + BIG_EPSILON);

      const float h = std::min(projH, static_cast<float>(SCREEN_HEIGHT) * 2.f);

      const float columnX = static_cast<float>(i) * columnWidth;
      const float columnY = (static_cast<float>(SCREEN_HEIGHT) / 2.f) - (h / 2.f);


      float     brightnessFactor = 1.f - std::min(correctedDepth / (tileSize * MAX_LINEAR_ATTENUATION_DISTANCE), 1.f);
      uint8_t   col              = static_cast<uint8_t>(std::clamp(brightnessFactor * 255.f, 30.f, 255.f));
      sf::Color wallColor(col, col, col);

      sf::RectangleShape column(sf::Vector2f(std::ceil(columnWidth), h));
      column.setPosition(columnX, columnY);
      column.setFillColor(wallColor);
      window.draw(column);

      rayAngle += DELTA_ANGLE;
    }
  }
}