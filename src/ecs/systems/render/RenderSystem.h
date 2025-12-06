//
// Created by obamium3157 on 14.11.2025.
//

#ifndef NULLP0INT_RENDERSYSTEM_H
#define NULLP0INT_RENDERSYSTEM_H

#include <vector>
#include <SFML/Graphics/RenderWindow.hpp>

#include "TextureManager.h"
#include "../../Components.h"
#include "../../Registry.h"
#include "../../../configuration/Configuration.h"

namespace ecs
{
  struct RenderItem
  {
    float depth = 0.f;
    sf::Sprite sprite;
  };

  class RenderSystem
  {
  public:
    static void render(Registry &registry, const Configuration &config, sf::RenderWindow &window, const Entity &m_tilemap, float globalTime, const TextureManager &textureManager);

  private:
    static void renderFloor(Registry &registry, Configuration config, sf::RenderWindow &window, const Entity& m_tilemap, float playerRotationAngle, sf::Vector2f playerPos, const TextureManager& textureManager);
    static void renderWalls(Registry &registry, Configuration config, const Entity& tilemapEntity, float playerAngle, const RayCastResultComponent& rayResults, float globalTime, const TextureManager& textureManager, std::vector<RenderItem>& items);
    static void drawSolidColumn(sf::RenderWindow& window, float x, float y, float width, float height, float depth, float maxAttenuationDist);
    static void renderEnemies(Registry &registry, const Configuration &config, const TextureManager &textureManager, std::vector<RenderItem>& items);
  };
}

#endif //NULLP0INT_RENDERSYSTEM_H
