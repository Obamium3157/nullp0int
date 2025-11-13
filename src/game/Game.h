//
// Created by obamium3157 on 13.11.2025.
//

#ifndef NULLP0INT_GAME_H
#define NULLP0INT_GAME_H

#include <string>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>

#include "PlayerFactory.h"
#include "WallFactory.h"
#include "../ecs/Components.h"
#include "../ecs/Registry.h"
#include "../ecs/Systems.h"

class Game
{
public:
  explicit Game(const unsigned windowW = 1920, const unsigned windowH = 1080, const std::string& title = "NULLP0INT", const sf::ContextSettings &settings = sf::ContextSettings())
    : m_window(sf::VideoMode(windowW, windowH), title, sf::Style::Default, settings)
  {
    m_window.setFramerateLimit(60);
    init();
  }

  void run()
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

private:
  sf::RenderWindow m_window;
  ecs::Registry m_registry;

  ecs::RenderSystem m_renderSystem;
  ecs::PhysicsSystem m_physicsSystem;

  ecs::Entity m_player = ecs::INVALID_ENTITY;

  void init()
  {
    m_player = initPlayer(m_registry, {100.f, 100.f}, sf::Color::Cyan);

    for (int i = 0; i < 30; ++i)
    {
      initWall(m_registry, {50.f * i + 200.f, 300.f}, {25, 75}, sf::Color::Magenta);
    }

    // Здесь можно загрузить текстуры/шрифты и добавить render-компоненты и т.д.
  }

  void handleEvents()
  {
    sf::Event event{};
    while (m_window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
      {
        m_window.close();
      }
    }

    if (m_player != ecs::INVALID_ENTITY)
    {
      if (auto* vel = m_registry.getComponent<ecs::VelocityComponent>(m_player))
      {
        constexpr float speed = 150.f;
        vel->velocity = {};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  vel->velocity.x = -speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) vel->velocity.x = speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    vel->velocity.y = -speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  vel->velocity.y =  speed;
      }
    }
  }

  void update(const float dt)
  {
    ecs::PhysicsSystem::update(m_registry, dt);
  }

  void render()
  {
    m_window.clear(sf::Color::Black);
    ecs::RenderSystem::render(m_registry, m_window);
    m_window.display();
  }
};

#endif //NULLP0INT_GAME_H