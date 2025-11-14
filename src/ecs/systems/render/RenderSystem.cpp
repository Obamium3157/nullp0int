//
// Created by obamium3157 on 14.11.2025.
//

#include "RenderSystem.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "../../Components.h"
#include "../../Registry.h"

void ecs::RenderSystem::render(Registry &registry, sf::RenderWindow &window)
{
  Entity mapEntity = INVALID_ENTITY;
  for (const auto &e : registry.entities())
  {
    if (registry.hasComponent<TilemapComponent>(e))
    {
      mapEntity = e;
      break;
    }
  }

  const TilemapComponent* map = nullptr;
  if (mapEntity != INVALID_ENTITY)
  {
    map = registry.getComponent<TilemapComponent>(mapEntity);
  }

  if (map)
  {
    const float ts = map->tileScale;
    for (unsigned y = 0; y < map->height; ++y)
    {
      for (unsigned x = 0; x < map->width; ++x)
      {
        if (map->isWall(static_cast<int>(x), static_cast<int>(y)))
        {
          sf::RectangleShape rect(sf::Vector2f(ts, ts));
          rect.setPosition(static_cast<float>(x) * ts, static_cast<float>(y) * ts);
          rect.setFillColor(sf::Color::White);
          window.draw(rect);
        }
      }
    }
  }

  for (const auto &e : registry.entities())
  {
    auto* pos = registry.getComponent<PositionComponent>(e);
    if (!pos) continue;

    if (registry.hasComponent<RayCastResultComponent>(e))
    {
      if (auto* rr = registry.getComponent<RayCastResultComponent>(e); rr && !rr->hits.empty())
      {
        sf::VertexArray va(sf::Lines);
        for (const auto &hit : rr->hits)
        {
          va.append(sf::Vertex(pos->position, sf::Color::Yellow));
          va.append(sf::Vertex(hit.hitPointWorld, sf::Color::Yellow));
        }
        window.draw(va);
      }
    }

    if (registry.hasComponent<PlayerTag>(e))
    {
      const auto* color = registry.getComponent<ColorComponent>(e);
      const auto* radius = registry.getComponent<RadiusComponent>(e);
      const auto* rotation = registry.getComponent<RotationComponent>(e);
      if (!color || !radius || !rotation) continue;

      sf::CircleShape player(radius->radius);
      player.setOrigin(radius->radius, radius->radius);
      player.setPosition(pos->position);
      player.setRotation(rotation->angle);
      player.setFillColor(color->color);

      window.draw(player);
    }

  }
}