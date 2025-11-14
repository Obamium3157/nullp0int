//
// Created by obamium3157 on 13.11.2025.
//

#ifndef NULLP0INT_GAME_H
#define NULLP0INT_GAME_H

#include <string>
#include <SFML/Graphics/RenderWindow.hpp>
#include "../ecs/Registry.h"

class Game
{
public:
  explicit Game(unsigned windowW = 1920,
                unsigned windowH = 1080,
                const std::string& title = "NULLP0INT",
                unsigned antialiasing = 0);

  void run();

private:
  sf::RenderWindow m_window;
  ecs::Registry m_registry;

  ecs::Entity m_player = ecs::INVALID_ENTITY;
  ecs::Entity m_tilemap = ecs::INVALID_ENTITY;

  void init();
  void handleEvents();
  void update(float dt);
  void render();
};

#endif //NULLP0INT_GAME_H