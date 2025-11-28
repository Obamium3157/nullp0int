//
// Created by obamium3157 on 14.11.2025.
//

#ifndef NULLP0INT_MAPLOADERSYSTEM_H
#define NULLP0INT_MAPLOADERSYSTEM_H
#include <string>
#include <SFML/System/Vector2.hpp>

#include "../../Components.h"
#include "../../Entity.h"
#include "../../Registry.h"
#include "../../../configuration/Configuration.h"

namespace ecs
{
  class MapLoaderSystem
  {
  public:
    static Entity load(Registry& registry, Configuration config, const std::string& filename);
  };

  sf::Vector2f getMapPosition(sf::Vector2f position);

  bool insideMapIs(const TilemapComponent* map, int x, int y);
}



#endif //NULLP0INT_MAPLOADERSYSTEM_H