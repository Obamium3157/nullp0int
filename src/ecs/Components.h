//
// Created by obamium3157 on 13.11.2025.
//

#pragma once

#ifndef NULLP0INT_COMPONENTS_H
#define NULLP0INT_COMPONENTS_H

#include <SFML/Graphics/RenderWindow.hpp>

namespace ecs
{
  struct PositionComponent
  {
    sf::Vector2f position{};
  };

  struct SizeComponent
  {
    sf::Vector2f size{};
  };
  struct RadiusComponent
  {
    float radius = 0.f;
  };

  struct VelocityComponent
  {
    sf::Vector2f velocity{};
  };
  struct RotationVelocityComponent
  {
    float rotationVelocity = 0.f;
  };

  struct SpeedComponent
  {
    float speed = 0.f;
  };
  struct RotationSpeedComponent
  {
    float rotationSpeed = 0.f;
  };

  struct RotationComponent
  {
    float angle = 0.f;
  };

  struct ColorComponent
  {
    sf::Color color = sf::Color::White;
  };

  struct PlayerTag{};
  struct PlayerInput
  {
    float moveSpeed = 150.f;
  };


  struct TilemapTag{};
  struct TilemapComponent
  {
    unsigned width = 0;
    unsigned height = 0;

    float tileScale = 64.f;

    std::vector<std::string> tiles;

    [[nodiscard]] bool isWall(const int tx, const int ty) const
    {
      if (tx < 0 || ty < 0 || tx >= static_cast<int>(width) || ty >= static_cast<int>(height))
        return true;

      return tiles[ty][tx] == '#';
    }

    [[nodiscard]] sf::Vector2i worldToTile(const sf::Vector2f worldPos) const
    {
      return {static_cast<int>(worldPos.x / tileScale), static_cast<int>(worldPos.y / tileScale)};
    }
  };
}

#endif //NULLP0INT_COMPONENTS_H