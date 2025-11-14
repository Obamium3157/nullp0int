//
// Created by obamium3157 on 14.11.2025.
//

#include "RenderSystem.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <cmath>

#include "../../Components.h"
#include "../../Registry.h"
#include "../../../math/mathUtils.h"

void ecs::RenderSystem::render(Registry &registry, sf::RenderWindow &window)
{
  for (const auto &ents = registry.entities(); const auto &e : ents)
  {
    if (registry.getComponent<PlayerTag>(e))
    {
      const auto* pos = registry.getComponent<PositionComponent>(e);
      const auto* color = registry.getComponent<ColorComponent>(e);
      const auto* radius = registry.getComponent<RadiusComponent>(e);
      const auto* rotation = registry.getComponent<RotationComponent>(e);
      if (!pos || !color || !radius || !rotation) continue;

      sf::CircleShape player(radius->radius);
      player.setOrigin(radius->radius, radius->radius);
      player.setPosition(pos->position);
      player.setRotation(rotation->angle);
      player.setFillColor(color->color);

      window.draw(player);

      const     float    rad = radiansFromDegrees(rotation->angle);
      constexpr float    len = 2000.f;
      const sf::Vector2f dir { std::cos(rad), std::sin(rad) };
      const sf::Vector2f start = pos->position;
      const sf::Vector2f end   = start + dir * len;

      sf::Vertex line[] =
      {
        sf::Vertex(start, sf::Color::Yellow),
        sf::Vertex(end, sf::Color::Yellow),
      };

      window.draw(line, 2, sf::Lines);

    }
    else if (registry.getComponent<TilemapTag>(e))
    {
      const auto* tileStringVec = registry.getComponent<TilemapComponent>(e);
      const auto* tileScale = registry.getComponent<TilemapComponent>(e);
      if (!tileStringVec || !tileScale) continue;

      unsigned y = 0;
      for (const auto& tileString : tileStringVec->tiles)
      {
        unsigned x = 0;
        for (const auto& ch : tileString)
        {

          switch (ch)
          {
            case '#':
            {
              sf::RectangleShape tile({tileScale->tileScale, tileScale->tileScale});
              tile.setPosition({static_cast<float>(x) * tileScale->tileScale,
                static_cast<float>(y) * tileScale->tileScale});
              tile.setFillColor(sf::Color::White);

              window.draw(tile);
              break;
            }
            case ' ':
            {
              break;
            }
            default:
              break;
          }
          x++;
        }
        y++;
      }
    }

  }
}
