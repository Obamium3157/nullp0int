//
// Created by obamium3157 on 13.11.2025.
//

#include "Systems.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "Components.h"
#include "Registry.h"

void ecs::RenderSystem::render(Registry &registry, sf::RenderWindow &window)
{
  for (const auto &ents = registry.entities(); const auto &e : ents)
  {
    const auto* pos = registry.getComponent<PositionComponent>(e);
    if (!pos) continue;
    const auto* color = registry.getComponent<ColorComponent>(e);
    if (!color) continue;

    if (registry.getComponent<PlayerTag>(e))
    {
      sf::CircleShape player(25.f);
      player.setPosition(pos->position);
      player.setFillColor(color->color);

      window.draw(player);
    } else if (registry.getComponent<WallTag>(e))
    {
      const auto* size = registry.getComponent<SizeComponent>(e);
      sf::RectangleShape rect(size->size);
      rect.setPosition(pos->position);
      rect.setFillColor(color->color);

      window.draw(rect);
    }

  }
}

void ecs::PhysicsSystem::update(Registry &registry, const float deltaTime)
{
  for (const auto &ents = registry.entities(); const auto &e : ents)
  {
    auto* pos = registry.getComponent<PositionComponent>(e);

    if (const auto* vel = registry.getComponent<VelocityComponent>(e); pos && vel)
    {
      pos->position += vel->velocity * deltaTime;
    }
  }
}
