//
// Created by obamium3157 on 13.11.2025.
//

#include "Game.h"
#include "entities/player/PlayerFactory.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>

#include "../constants.h"
#include "../ecs/systems/render/RenderSystem.h"
#include "../ecs/systems/physics/PhysicsSystem.h"
#include "../ecs/systems/input/InputSystem.h"
#include "../ecs/systems/map/MapLoaderSystem.h"

Game::Game(const unsigned windowW, const unsigned windowH, const std::string &title, const unsigned antialiasing)
  : m_window(sf::VideoMode(windowW, windowH), title, sf::Style::Default,
             sf::ContextSettings{0, 0, antialiasing})
{
  m_window.setFramerateLimit(60);
  init();
}


void Game::run()
{
  sf::Clock clock;
  while (m_window.isOpen())
  {
    const float dt = clock.restart().asSeconds();
    handleEvents();
    update(dt);
    render();
  }
}

void Game::init()
{
  m_player = initPlayer(m_registry, PLAYER_INITIAL_POSITION, PLAYER_RADIUS, PLAYER_SPEED, PLAYER_ROTATION_SPEED, PLAYER_COLOR);

  m_tilemap = ecs::MapLoaderSystem::load(m_registry, "resources/maps/map1.txt");

  // Здесь можно загрузить текстуры/шрифты и добавить render-компоненты и т.д.
}

void Game::handleEvents()
{
  sf::Event event{};
  while (m_window.pollEvent(event))
  {
    if (event.type == sf::Event::Closed)
    {
      m_window.close();
    }
  }
}

void Game::update(const float dt)
{
  ecs::InputSystem::update(m_registry);
  ecs::PhysicsSystem::update(m_registry, dt, m_tilemap);
}

void Game::render()
{
  m_window.clear(sf::Color::Black);
  ecs::RenderSystem::render(m_registry, m_window);
  m_window.display();
}

