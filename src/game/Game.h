//
// Created by obamium3157 on 13.11.2025.
//

#ifndef NULLP0INT_GAME_H
#define NULLP0INT_GAME_H

#include <string>
#include <SFML/Graphics/RenderWindow.hpp>

#include "../constants.h"
#include "../configuration/Configuration.h"
#include "../ecs/Registry.h"
#include "../ecs/systems/render/TextureManager.h"

class Game
{
public:
  explicit Game(unsigned windowW = SCREEN_WIDTH,
                unsigned windowH = SCREEN_HEIGHT,
                const std::string& title = "NULLP0INT",
                unsigned antialiasing = 0);

  void run();

private:
  sf::RenderWindow m_window;
  ecs::Registry m_registry;
  TextureManager m_textureManager;

  Configuration m_config;

  ecs::Entity m_player = ecs::INVALID_ENTITY;
  ecs::Entity m_tilemap = ecs::INVALID_ENTITY;

  sf::Clock m_globalTimer;

  void init();
  void handleEvents();
  void update(float dt);
  void render();
};

#endif //NULLP0INT_GAME_H