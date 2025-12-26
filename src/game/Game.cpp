//
// Created by obamium3157 on 13.11.2025.
//

#include "Game.h"
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include "entities/player/PlayerFactory.h"

#include "../ecs/systems/input/InputSystem.h"
#include "../ecs/systems/map/MapLoaderSystem.h"
#include "../ecs/systems/map_generation/MapGenerationSystem.h"
#include "../ecs/systems/npc/EnemyControllerSystem.h"
#include "../ecs/systems/npc/PathfindingSystem.h"
#include "../ecs/systems/physics/PhysicsSystem.h"
#include "../ecs/systems/render/AnimationSystem.h"
#include "../ecs/systems/render/RayCasting.h"
#include "../ecs/systems/render/RenderSystem.h"
#include "../ecs/systems/render/TextureManager.h"
#include "entities/enemy/EnemyFactory.h"

Game::Game(const unsigned windowW, const unsigned windowH, const std::string &title, const unsigned antialiasing)
  : m_window(sf::VideoMode(windowW, windowH),title,
    sf::Style::Default,
    sf::ContextSettings{ 0, 0, antialiasing }
  )
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

void Game::init_tilemap(const uint32_t seed)
{
  // MapGenerationSystem mgs{100, 100, seed};
  // const std::string filename = mgs.generateLevel(50);
  // m_tilemap = ecs::MapLoaderSystem::load(m_registry, m_config, "resources/maps/generated/" + filename);
  m_tilemap = ecs::MapLoaderSystem::load(m_registry, m_config, "resources/maps/map2.txt");
}

void Game::init_textures()
{
  m_textureManager.load("wall_texture", "resources/assets/DOOR2_4.png");
  m_textureManager.load("floor", "resources/assets/FLAT5_8.png");
  m_textureManager.load("step1", "resources/assets/STEP1.png");
  m_textureManager.load("step2", "resources/assets/STEP2.png");
  m_textureManager.load("sinner", "resources/assets/WALL50_1.png");
  m_textureManager.load("nwall", "resources/assets/COMP02_5.png");
  m_textureManager.load("1wall", "resources/assets/MFLR8_1.png");
  m_textureManager.load("2wall", "resources/assets/MFLR8_3.png");

  m_textureManager.load("placeholder", "resources/assets/FCANA0.png");

  // m_textureManager.load("melee_enemy", "resources/assets/BOSSA1.png");

  m_textureManager.load("melee_enemy_1", "resources/assets/BOSSA1.png");
  m_textureManager.load("melee_enemy_2", "resources/assets/BOSSC1.png");

  m_textureManager.load("range_enemy", "resources/assets/PLAYA1.png");
  m_textureManager.load("support_enemy", "resources/assets/HEADD1.png");


  if (auto* tm = m_registry.getComponent<ecs::TilemapComponent>(m_tilemap))
  {
    tm->tileAppearanceMap['#'] = {"wall_texture", {}};
    tm->tileAppearanceMap['P'] = {"step1", {"step1", "step2"}, 0.5f};
    tm->tileAppearanceMap['p'] = {"step1", {"step1", "step2"}, 0.5f};
    tm->tileAppearanceMap['S'] = {"sinner", {}};
    tm->tileAppearanceMap['s'] = {"sinner", {}};
    tm->tileAppearanceMap['N'] = { "nwall", {} };
    tm->tileAppearanceMap['n'] = { "nwall", {} };
    tm->tileAppearanceMap['1'] = { "1wall", {} };
    tm->tileAppearanceMap['2'] = { "2wall", {} };
    tm->floorTextureId = "floor";
  }
}

void Game::init_player()
{
  const auto player_radius = m_config.player_radius;
  const auto player_speed = m_config.player_speed;
  const auto player_rotation_speed = m_config.player_rotation_speed;

  sf::Vector2f playerInitialPosition;
  if (auto* tm = m_registry.getComponent<ecs::TilemapComponent>(m_tilemap))
  {
    playerInitialPosition = tm->getSpawnPosition();
  }


  m_player = initPlayer(m_registry, playerInitialPosition, player_radius, player_speed, player_rotation_speed);
}

void Game::init()
{
  init_tilemap(9019);
  init_textures();
  init_player();
  spawnEnemiesFromMap(m_registry, m_tilemap, m_config);

  // if (m_player != ecs::INVALID_ENTITY)
  // {
  //   if (auto* ppos = m_registry.getComponent<ecs::PositionComponent>(m_player))
  //   {
  //     constexpr float offsetTiles = 3.f;
  //     const sf::Vector2f enemyPos = ppos->position + sf::Vector2f(m_config.tile_size * offsetTiles, 0.f);
  //     const sf::Vector2f enemyPos1 = ppos->position + sf::Vector2f(m_config.tile_size * offsetTiles, 0.f) * 2.f;
  //
  //     const float enemyRadius = m_config.player_radius;
  //     const float enemySpeed = m_config.player_speed * 0.5f;
  //
  //     initEnemy(m_registry, ecs::EnemyClass::RANGE, enemyPos, enemyRadius, enemySpeed);
  //     initEnemy(m_registry, ecs::EnemyClass::MELEE, enemyPos1, enemyRadius, enemySpeed);
  //   }
  // }
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
  ecs::InputSystem::update(m_registry, m_config);
  ecs::PathfindingSystem::update(m_registry, m_tilemap);
  // EnemyControllerSystem::update(m_registry);
  ecs::AnimationSystem::update(m_registry, dt);
  ecs::PhysicsSystem::update(m_registry, dt, m_tilemap);
  ecs::RayCasting::rayCast(m_registry, m_config, m_player);
}

void Game::render()
{
  const float globalTime = m_globalTimer.getElapsedTime().asSeconds();
  m_window.clear(sf::Color::Black);
  ecs::RenderSystem::render(m_registry, m_config, m_window, m_tilemap, globalTime, m_textureManager);
  m_window.display();
}

