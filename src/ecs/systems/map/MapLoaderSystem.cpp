//
// Created by obamium3157 on 14.11.2025.
//

#include "MapLoaderSystem.h"

#include <fstream>
#include <iostream>
#include <vector>

#include "../../Components.h"
#include "../../Registry.h"
#include "../../../constants.h"

ecs::Entity ecs::MapLoaderSystem::load(Registry &registry, const std::string &filename)
{
  std::ifstream file(filename);
  if (!file.is_open())
  {
    std::cerr << "Failed to open the file: " << filename << std::endl;
    return INVALID_ENTITY;
  }

  std::cout << "found the file lol\n";

  std::string line;
  std::vector<std::string> strMap;
  while (std::getline(file, line))
  {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line.empty()) continue;
    strMap.push_back(line);
  }

  if (strMap.empty())
  {
    std::cerr << "Map file is empty: " << filename << std::endl;
    return INVALID_ENTITY;
  }

  const Entity    mapEntity = registry.createEntity();
  const auto      width     = static_cast<unsigned>(strMap[0].length());
  const auto      height    = static_cast<unsigned>(strMap.size());
  registry.addComponent<TilemapComponent>(mapEntity, TilemapComponent{width, height, TILE_SCALE, strMap});
  registry.addComponent<TilemapTag>(mapEntity, TilemapTag{});

  return mapEntity;
}

sf::Vector2f ecs::getMapPosition(const sf::Vector2f position)
{
  return static_cast<sf::Vector2f>(static_cast<sf::Vector2i>(position));
}

bool ecs::insideMapIs(const TilemapComponent *map, const int x, const int y)
{
  if (!map) return false;
  return x >= 0 && y >= 0 && x < static_cast<int>(map->width) && y < static_cast<int>(map->height);
}
