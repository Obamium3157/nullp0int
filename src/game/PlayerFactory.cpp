//
// Created by obamium3157 on 13.11.2025.
//

#include "PlayerFactory.h"

#include "../ecs/Components.h"

ecs::Entity initPlayer(ecs::Registry &registry, const sf::Vector2f initialPos, const sf::Color color)
{
  const ecs::Entity player = registry.createEntity();
  registry.addComponent<ecs::PositionComponent>(player, ecs::PositionComponent{initialPos});
  registry.addComponent<ecs::VelocityComponent>(player, ecs::VelocityComponent{});
  registry.addComponent<ecs::ColorComponent>(player, ecs::ColorComponent{color});
  registry.addComponent<ecs::PlayerTag>(player, ecs::PlayerTag{});

  return player;
}
