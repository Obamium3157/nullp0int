//
// Created by obamium3157 on 13.11.2025.
//

#include "PlayerFactory.h"

#include "../../../ecs/Components.h"

ecs::Entity initPlayer(ecs::Registry &registry, const sf::Vector2f initialPos, const float radius, const float speed, const float rotationSpeed)
{
  const ecs::Entity player = registry.createEntity();
  registry.addComponent<ecs::PositionComponent>(player, ecs::PositionComponent{initialPos});
  registry.addComponent<ecs::RadiusComponent>(player, ecs::RadiusComponent{radius});
  registry.addComponent<ecs::RotationComponent>(player, ecs::RotationComponent{});
  registry.addComponent<ecs::VelocityComponent>(player, ecs::VelocityComponent{});
  registry.addComponent<ecs::RotationVelocityComponent>(player, ecs::RotationVelocityComponent{});
  registry.addComponent<ecs::SpeedComponent>(player, ecs::SpeedComponent{speed});
  registry.addComponent<ecs::RotationSpeedComponent>(player, ecs::RotationSpeedComponent{rotationSpeed});
  registry.addComponent<ecs::PlayerTag>(player, ecs::PlayerTag{});
  registry.addComponent<ecs::PlayerInput>(player, ecs::PlayerInput{});
  registry.addComponent<ecs::HealthComponent>(player, ecs::HealthComponent{100.f, 100.f});

  return player;
}
