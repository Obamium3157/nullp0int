//
// Created by obamium3157 on 13.11.2025.
//

#include "Game.h"

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
  m_player = initPlayer(m_registry, {100.f, 100.f}, sf::Color::Cyan);

  for (int i = 0; i < 30; ++i)
  {
    initWall(m_registry, {50.f * i + 200.f, 300.f}, {25, 75}, sf::Color::Magenta);
  }

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

void Game::update(const float dt)
{
  ecs::PhysicsSystem::update(m_registry, dt);
}

void Game::render()
{
  m_window.clear(sf::Color::Black);
  ecs::RenderSystem::render(m_registry, m_window);
  m_window.display();
}

