//
// Created by obamium3157 on 14.11.2025.
//

#include "InputSystem.h"

#include <cmath>
#include <SFML/Window/Keyboard.hpp>

#include "../../Components.h"
#include "../../../constants.h"
#include "../../../math/mathUtils.h"

void ecs::InputSystem::update(Registry &registry, const Configuration &config)
{
  for (const auto &ents = registry.entities(); const auto &e: ents)
  {
    if (!registry.hasComponent<PlayerInput>(e)) continue;

    const auto* input = registry.getComponent<PlayerInput>(e);
    const auto* rotationComp = registry.getComponent<RotationComponent>(e);
    const auto* rotationSpeedComp = registry.getComponent<RotationSpeedComponent>(e);
    auto* velocityComp = registry.getComponent<VelocityComponent>(e);
    auto* rotationVelocityComp = registry.getComponent<RotationVelocityComponent>(e);
    if (!input || !rotationComp || !rotationSpeedComp || !velocityComp || !rotationVelocityComp) continue;

    velocityComp->velocity.x = 0.f;
    velocityComp->velocity.y = 0.f;
    rotationVelocityComp->rotationVelocity = 0.f;

    const float moveSpeed = input->moveSpeed;
    const float rotationSpeed = rotationSpeedComp->rotationSpeed;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) rotationVelocityComp->rotationVelocity -= rotationSpeed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) rotationVelocityComp->rotationVelocity += rotationSpeed;

    const float rad = radiansFromDegrees(rotationComp->angle);
    const sf::Vector2f forward = { std::cos(rad), std::sin(rad) };
    const sf::Vector2f right{ forward.y * -1.f, forward.x };

    float moveForward = 0.f;
    float moveRight   = 0.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) moveForward += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) moveRight   -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) moveForward -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) moveRight   += 1.f;

    sf::Vector2f dir = forward * moveForward + right * moveRight;

    if (const float lenSq = dir.x * dir.x + dir.y * dir.y; lenSq > BIG_EPSILON)
    {
      const float len = std::sqrt(lenSq);
      dir.x /= len;
      dir.y /= len;
      velocityComp->velocity = dir * moveSpeed * velocityComp->velocityMultiplier;
    }
    else
    {
      velocityComp->velocity = {};
    }
    velocityComp->velocityMultiplier = (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) ? config.player_velocity_multiplier : 1.f;
  }
}
