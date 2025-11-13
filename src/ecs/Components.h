//
// Created by obamium3157 on 13.11.2025.
//

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

  struct VelocityComponent
  {
    sf::Vector2f velocity{};
  };

  struct ColorComponent
  {
    sf::Color color = sf::Color::White;
  };


}

#endif //NULLP0INT_COMPONENTS_H