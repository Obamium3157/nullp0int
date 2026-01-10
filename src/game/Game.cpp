#include "Game.h"
#include "GameUI.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <random>
#include <vector>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>

#include "entities/enemy/EnemyFactory.h"
#include "entities/player/PlayerFactory.h"

#include "../ecs/Components.h"
#include "../ecs/systems/input/InputSystem.h"
#include "../ecs/systems/map/MapLoaderSystem.h"
#include "../ecs/systems/map_generation/MapGenerationSystem.h"
#include "../ecs/systems/npc/PathfindingSystem.h"
#include "../ecs/systems/physics/PhysicsSystem.h"
#include "../ecs/systems/projectile/ProjectileSystem.h"
#include "../ecs/systems/render/AnimationSystem.h"
#include "../ecs/systems/render/RayCasting.h"
#include "../ecs/systems/render/RenderSystem.h"
#include "../ecs/systems/weapon/WeaponSystem.h"

Game::Game(const unsigned windowW, const unsigned windowH, const std::string& title, const unsigned antialiasing)
  : m_window(
      sf::VideoMode(windowW, windowH),
      title,
      sf::Style::Fullscreen,
      sf::ContextSettings{0, 0, antialiasing}
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

void Game::init()
{
  m_uiFontLoaded = loadUiFont(m_uiFont);
  setState(GlobalState::MainMenu);

  setMouseCaptured(m_window, false);
}

void Game::init_tilemap(const uint32_t seed, const MapChoice choice)
{
  if (choice == MapChoice::TestMap)
  {
    m_tilemap = ecs::MapLoaderSystem::load(m_registry, m_config, "resources/maps/map2.txt");
    return;
  }

  MapGenerationSystem mgs{100, 100, seed};
  const std::string filename = mgs.generateLevel(50);
  m_tilemap = ecs::MapLoaderSystem::load(m_registry, m_config, "resources/maps/generated/" + filename);
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

  m_textureManager.load("pistol_idle", "resources/assets/PISGA0.png");
  m_textureManager.load("pistol_fire_0", "resources/assets/PISGB0.png");
  m_textureManager.load("pistol_fire_1", "resources/assets/PISGC0.png");
  m_textureManager.load("pistol_fire_2", "resources/assets/PISGD0.png");
  m_textureManager.load("pistol_fire_3", "resources/assets/PISGE0.png");

  m_textureManager.load("shotgun_idle", "resources/assets/SHTGA0.png");
  m_textureManager.load("shotgun_fire_0", "resources/assets/SHTGA0.png");
  m_textureManager.load("shotgun_fire_1", "resources/assets/SHTGB0.png");
  m_textureManager.load("shotgun_fire_2", "resources/assets/SHTGC0.png");
  m_textureManager.load("shotgun_fire_3", "resources/assets/SHTGD0.png");

  m_textureManager.load("shotgun_projectile", "resources/assets/BAL1A0.png");
  m_textureManager.load("support_projectile", "resources/assets/IFOGA0.png");

  m_textureManager.load("parry1", "resources/assets/PUNGB0.png");
  m_textureManager.load("parry2", "resources/assets/PUNGC0.png");
  m_textureManager.load("parry3", "resources/assets/PUNGD0.png");
  m_textureManager.load("parried_projectile", "resources/assets/APBXA0.png");

  m_textureManager.load("melee_walk_1", "resources/assets/BOSSA1.png");
  m_textureManager.load("melee_walk_2", "resources/assets/BOSSC1.png");
  m_textureManager.load("melee_attack_1", "resources/assets/BOSSE1.png");
  m_textureManager.load("melee_attack_2", "resources/assets/BOSSF1.png");
  m_textureManager.load("melee_attack_3", "resources/assets/BOSSG1.png");

  m_textureManager.load("range_walk_1", "resources/assets/PLAYA1.png");
  m_textureManager.load("range_walk_2", "resources/assets/PLAYC1.png");
  m_textureManager.load("range_walk_left_1", "resources/assets/PLAYA3A7R.png");
  m_textureManager.load("range_walk_left_2", "resources/assets/PLAYA4A6R.png");
  m_textureManager.load("range_walk_right_1", "resources/assets/PLAYA4A6.png");
  m_textureManager.load("range_walk_right_2", "resources/assets/PLAYA3A7.png");
  m_textureManager.load("range_walk_back_1", "resources/assets/PLAYB5.png");
  m_textureManager.load("range_walk_back_2", "resources/assets/PLAYD5.png");
  m_textureManager.load("range_attack_1", "resources/assets/PLAYE1.png");
  m_textureManager.load("range_attack_2", "resources/assets/PLAYF1.png");

  m_textureManager.load("support_walk_1", "resources/assets/SPOSA1.png");
  m_textureManager.load("support_walk_2", "resources/assets/SPOSB1.png");
  m_textureManager.load("support_walk_3", "resources/assets/SPOSC1.png");
  m_textureManager.load("support_walk_4", "resources/assets/SPOSD1.png");
  m_textureManager.load("support_walk_left_1", "resources/assets/SPOSA2A8R.png");
  m_textureManager.load("support_walk_left_2", "resources/assets/SPOSA3A7R.png");
  m_textureManager.load("support_walk_left_3", "resources/assets/SPOSA4A6R.png");
  m_textureManager.load("support_walk_right_1", "resources/assets/SPOSA2A8.png");
  m_textureManager.load("support_walk_right_2", "resources/assets/SPOSA3A7.png");
  m_textureManager.load("support_walk_right_3", "resources/assets/SPOSA4A6.png");
  m_textureManager.load("support_walk_back_1", "resources/assets/SPOSB5.png");
  m_textureManager.load("support_walk_back_2", "resources/assets/SPOSC5.png");
  m_textureManager.load("support_attack_1", "resources/assets/SPOSE1.png");
  m_textureManager.load("support_attack_2", "resources/assets/SPOSF1.png");

  if (auto* tm = m_registry.getComponent<ecs::TilemapComponent>(m_tilemap))
  {
    tm->tileAppearanceMap['#'] = {"wall_texture", {}};
    tm->tileAppearanceMap['P'] = {"step1", {"step1", "step2"}, 0.5f};
    tm->tileAppearanceMap['p'] = {"step1", {"step1", "step2"}, 0.5f};
    tm->tileAppearanceMap['S'] = {"sinner", {}};
    tm->tileAppearanceMap['s'] = {"sinner", {}};
    tm->tileAppearanceMap['N'] = {"nwall", {}};
    tm->tileAppearanceMap['n'] = {"nwall", {}};
    tm->tileAppearanceMap['1'] = {"1wall", {}};
    tm->tileAppearanceMap['2'] = {"2wall", {}};
    tm->floorTextureId = "floor";
  }
}

void Game::init_player()
{
  const float player_radius = m_config.player_radius;
  const float player_speed = m_config.player_speed;
  const float player_rotation_speed = m_config.player_rotation_speed;

  sf::Vector2f playerInitialPosition;
  if (const auto* tm = m_registry.getComponent<ecs::TilemapComponent>(m_tilemap))
  {
    playerInitialPosition = tm->getSpawnPosition();
  }

  m_player = initPlayer(m_registry, playerInitialPosition, player_radius, player_speed, player_rotation_speed);
}

void Game::handleEvents()
{
  sf::Event event{};
  while (m_window.pollEvent(event))
  {
    if (event.type == sf::Event::Closed)
    {
      m_window.close();
      continue;
    }

    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
    {
      if (m_state == GlobalState::Playing)
      {
        setState(GlobalState::Paused);
      }
      else if (m_state == GlobalState::Paused)
      {
        setState(GlobalState::Playing);
      }
      else if (m_state == GlobalState::MapSelect)
      {
        setState(GlobalState::MainMenu);
      }
    }

    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
    {
      if (m_state == GlobalState::Playing) continue;

      const sf::Vector2f mp = m_window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
      const auto buttons = buildButtonsForState(m_state);
      for (std::size_t i = 0; i < buttons.size(); ++i)
      {
        if (buttons[i].rect.contains(mp))
        {
          onMenuButtonPressed(i);
          break;
        }
      }
    }
  }
}

void Game::update(const float dt)
{
  const float dtSafe = std::max(0.f, dt);

  const bool captureMouse = (m_state == GlobalState::Playing) && m_window.hasFocus();

  static bool s_captureApplied = false;
  if (captureMouse != s_captureApplied)
  {
    setMouseCaptured(m_window, captureMouse);
    s_captureApplied = captureMouse;
  }

  const float mouseDx = consumeMouseDeltaX(m_window, captureMouse);

  if (m_state == GlobalState::Playing)
  {
    m_worldTimeSeconds += dtSafe;

    ecs::InputSystem::update(m_registry, m_config, dtSafe, mouseDx);
    ecs::PathfindingSystem::update(m_registry, m_tilemap, dtSafe);
    ecs::AnimationSystem::update(m_registry, dtSafe);
    ecs::PhysicsSystem::update(m_registry, dtSafe, m_tilemap);
    ecs::RayCasting::rayCast(m_registry, m_config, m_player);
    ecs::WeaponSystem::update(m_registry, m_config, m_tilemap, m_player, dtSafe);
    ecs::ProjectileSystem::update(m_registry, m_config, m_tilemap, dtSafe);
  }

  if (m_state == GlobalState::Playing || m_state == GlobalState::Paused)
  {
    m_hud.update(dtSafe, m_registry, m_player);
  }

  if (m_state != GlobalState::Playing) return;

  if (m_player != ecs::INVALID_ENTITY)
  {
    if (auto* hp = m_registry.getComponent<ecs::HealthComponent>(m_player))
    {
      if (hp->current <= 0.f)
      {
        hp->current = 0.f;
        setState(GlobalState::DeathMenu);
      }
    }
  }
}

void Game::drawHud()
{
  m_hud.draw(m_window, m_registry, m_player, m_uiFont, m_uiFontLoaded);
}

void Game::render()
{
  m_window.clear(sf::Color::Black);

  const bool hasWorld =
    (m_state == GlobalState::Playing || m_state == GlobalState::Paused || m_state == GlobalState::DeathMenu);

  if (hasWorld && m_tilemap != ecs::INVALID_ENTITY)
  {
    ecs::RenderSystem::render(m_registry, m_config, m_window, m_tilemap, m_worldTimeSeconds, m_textureManager);

    if (m_state == GlobalState::Playing || m_state == GlobalState::Paused)
    {
      drawWeaponView();
      drawHud();
    }
  }

  switch (m_state)
  {
    case GlobalState::MainMenu:
      drawMenu("NULLP0INT", buildButtonsForState(m_state), false);
      break;

    case GlobalState::MapSelect:
      drawMenu("Выбор уровня", buildButtonsForState(m_state), false);
      break;

    case GlobalState::Paused:
      drawMenu("Пауза", buildButtonsForState(m_state), true);
      break;

    case GlobalState::DeathMenu:
      drawMenu("Вы умерли", buildButtonsForState(m_state), true);
      break;

    case GlobalState::Playing:
      break;
  }

  m_window.display();
}
