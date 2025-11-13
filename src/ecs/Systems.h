//
// Created by obamium3157 on 13.11.2025.
//

#ifndef NULLP0INT_SYSTEMS_H
#define NULLP0INT_SYSTEMS_H

#include <SFML/Graphics/RenderWindow.hpp>

namespace ecs
{
  class Registry;

  class RenderSystem
  {
  public:
    static void render(Registry& registry, sf::RenderWindow& window);
  };

  class PhysicsSystem
  {
  public:
    static void update(Registry& registry, float deltaTime);
  };
}

#endif //NULLP0INT_SYSTEMS_H