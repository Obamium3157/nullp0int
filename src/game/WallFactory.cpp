//
// Created by obamium3157 on 13.11.2025.
//

#include "WallFactory.h"

#include "../ecs/Components.h"

ecs::Entity initWall(ecs::Registry &registry, const sf::Vector2f pos, const sf::Vector2f size, const sf::Color fillColor)
{
  const auto e = registry.createEntity();
  registry.addComponent<ecs::PositionComponent>(e, ecs::PositionComponent{pos});
  registry.addComponent<ecs::SizeComponent>(e, ecs::SizeComponent{size});
  registry.addComponent<ecs::ColorComponent>(e, ecs::ColorComponent{fillColor});
  registry.addComponent<ecs::WallTag>(e, ecs::WallTag{});

  return e;
}
