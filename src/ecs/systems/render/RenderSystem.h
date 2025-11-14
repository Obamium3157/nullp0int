//
// Created by obamium3157 on 14.11.2025.
//

#ifndef NULLP0INT_RENDERSYSTEM_H
#define NULLP0INT_RENDERSYSTEM_H

#include <SFML/Graphics/RenderWindow.hpp>

#include "../../Registry.h"

namespace ecs
{
  class RenderSystem
  {
  public:
    static void render(Registry& registry, sf::RenderWindow& window);
  };
}


#endif //NULLP0INT_RENDERSYSTEM_H