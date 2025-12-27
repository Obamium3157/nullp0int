//
// Created by obamium3157 on 13.11.2025.
//

#include "Game.h"

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

namespace
{
  constexpr auto UI_FONT_PATH = "resources/fonts/tektur/Tektur-Black.ttf";

  [[nodiscard]] bool loadUiFont(sf::Font& font)
  {
    try
    {
      if (!std::filesystem::exists(UI_FONT_PATH)) return false;
    }
    catch (...)
    {
      return false;
    }

    return font.loadFromFile(UI_FONT_PATH);
  }

  [[nodiscard]] sf::String toSfStringUtf8(const std::string_view s)
  {
    return sf::String::fromUtf8(s.begin(), s.end());
  }

  [[nodiscard]] sf::FloatRect centeredRect(const sf::Vector2u winSize, const float y, const float w, const float h)
  {
    const float x = (static_cast<float>(winSize.x) - w) * 0.5f;
    return {x, y, w, h};
  }

  void setMouseCaptured(sf::RenderWindow& window, const bool captured)
  {
    window.setMouseCursorVisible(!captured);
    window.setMouseCursorGrabbed(captured);
  }

  [[nodiscard]] sf::Vector2i windowCenterPx(const sf::RenderWindow& window)
  {
    const auto sz = window.getSize();
    return { static_cast<int>(sz.x / 2u), static_cast<int>(sz.y / 2u) };
  }

  [[nodiscard]] float consumeMouseDeltaX(const sf::RenderWindow& window, const bool captureNow)
  {
    static bool s_prevCapture = false;

    if (!captureNow)
    {
      s_prevCapture = false;
      return 0.f;
    }

    const sf::Vector2i center = windowCenterPx(window);

    if (!s_prevCapture)
    {
      sf::Mouse::setPosition(center, window);
      s_prevCapture = true;
      return 0.f;
    }

    const sf::Vector2i pos = sf::Mouse::getPosition(window);
    const int dx = pos.x - center.x;

    sf::Mouse::setPosition(center, window);

    return static_cast<float>(dx);
  }

  void drawButton(sf::RenderWindow& window, const sf::Font& font, const UIButton& b, const bool fontLoaded)
  {
    const sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    const bool hovered = b.rect.contains(mousePos);

    sf::RectangleShape shape;
    shape.setPosition({b.rect.left, b.rect.top});
    shape.setSize({b.rect.width, b.rect.height});
    shape.setFillColor(hovered ? sf::Color(55, 55, 55, 235) : sf::Color(30, 30, 30, 235));
    shape.setOutlineThickness(2.f);
    shape.setOutlineColor(hovered ? sf::Color(200, 200, 200) : sf::Color(120, 120, 120));
    window.draw(shape);

    if (!fontLoaded) return;

    sf::Text text;
    text.setFont(font);
    text.setString(toSfStringUtf8(b.label));
    text.setCharacterSize(32);
    text.setFillColor(sf::Color::White);

    const sf::FloatRect bounds = text.getLocalBounds();
    const float tx = b.rect.left + (b.rect.width - bounds.width) * 0.5f - bounds.left;
    const float ty = b.rect.top + (b.rect.height - bounds.height) * 0.5f - bounds.top;
    text.setPosition(tx, ty);
    window.draw(text);
  }

  void drawTitle(sf::RenderWindow& window, const sf::Font& font, const std::string& title, const bool fontLoaded, const float y)
  {
    if (!fontLoaded) return;

    sf::Text text;
    text.setFont(font);
    text.setString(toSfStringUtf8(title));
    text.setCharacterSize(64);
    text.setFillColor(sf::Color::White);

    const sf::FloatRect b = text.getLocalBounds();
    const float x = (static_cast<float>(window.getSize().x) - b.width) * 0.5f - b.left;
    text.setPosition(x, y);
    window.draw(text);
  }
}

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

void Game::setState(const GlobalState next)
{
  m_state = next;
}

void Game::returnToMainMenu()
{
  m_registry = ecs::Registry{};
  m_player = ecs::INVALID_ENTITY;
  m_tilemap = ecs::INVALID_ENTITY;
  m_worldTimeSeconds = 0.f;
  setState(GlobalState::MainMenu);
}

void Game::startNewGame(const MapChoice choice)
{
  m_registry = ecs::Registry{};
  m_player = ecs::INVALID_ENTITY;
  m_tilemap = ecs::INVALID_ENTITY;
  m_worldTimeSeconds = 0.f;

  std::random_device rd;
  const auto seed = rd();

  init_tilemap(seed, choice);
  init_textures();
  init_player();
  spawnEnemiesFromMap(m_registry, m_tilemap, m_config);

  setState(GlobalState::Playing);
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
  m_textureManager.load("shotgun_fire_3", "resources/assets/SHTGA0.png");

  m_textureManager.load("shotgun_projectile", "resources/assets/BAL1A0.png");

  m_textureManager.load("melee_walk_1", "resources/assets/BOSSA1.png");
  m_textureManager.load("melee_walk_2", "resources/assets/BOSSC1.png");
  m_textureManager.load("melee_attack_1", "resources/assets/BOSSE1.png");
  m_textureManager.load("melee_attack_2", "resources/assets/BOSSF1.png");
  m_textureManager.load("melee_attack_3", "resources/assets/BOSSG1.png");

  m_textureManager.load("range_walk_1", "resources/assets/PLAYA1.png");
  m_textureManager.load("range_walk_2", "resources/assets/PLAYC1.png");
  m_textureManager.load("range_attack_1", "resources/assets/PLAYE1.png");
  m_textureManager.load("range_attack_2", "resources/assets/PLAYF1.png");

  m_textureManager.load("support_walk_1", "resources/assets/SPOSA1.png");
  m_textureManager.load("support_walk_2", "resources/assets/SPOSB1.png");
  m_textureManager.load("support_walk_3", "resources/assets/SPOSC1.png");
  m_textureManager.load("support_walk_4", "resources/assets/SPOSD1.png");
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

std::vector<UIButton> Game::buildButtonsForState(const GlobalState state) const
{
  const sf::Vector2u ws      = m_window.getSize();
  const float        btnW    = std::min(520.f, static_cast<float>(ws.x) * 0.55f);
  constexpr float    btnH    = 70.f;
  constexpr float    spacing = 50.f;

  std::vector<UIButton> out;

  auto push = [&](const float y, const std::string& label)
  {
    out.push_back(UIButton{centeredRect(ws, y, btnW, btnH), label});
  };

  switch (state)
  {
    case GlobalState::MainMenu:
      push(360.f, "Начать игру");
      push(360.f + (btnH + spacing), "Выход из игры");
      break;

    case GlobalState::MapSelect:
      push(360.f, "Процедурный уровень");
      push(360.f + (btnH + spacing), "Тестовая карта");
      break;

    case GlobalState::Paused:
      push(360.f, "Продолжить");
      push(360.f + (btnH + spacing), "Выход");
      break;

    case GlobalState::DeathMenu:
      push(360.f, "В главное меню");
      push(360.f + (btnH + spacing), "Выход из игры");
      break;

    case GlobalState::Playing:
      break;
  }

  return out;
}

void Game::onMenuButtonPressed(const std::size_t buttonIndex)
{
  switch (m_state)
  {
    case GlobalState::MainMenu:
      if (buttonIndex == 0)
      {
        setState(GlobalState::MapSelect);
      }
      else if (buttonIndex == 1)
      {
        m_window.close();
      }
      break;

    case GlobalState::MapSelect:
      if (buttonIndex == 0)
      {
        startNewGame(MapChoice::Procedural);
      }
      else if (buttonIndex == 1)
      {
        startNewGame(MapChoice::TestMap);
      }
      break;

    case GlobalState::Paused:
      if (buttonIndex == 0)
      {
        setState(GlobalState::Playing);
      }
      else if (buttonIndex == 1)
      {
        returnToMainMenu();
      }
      break;

    case GlobalState::DeathMenu:
      if (buttonIndex == 0)
      {
        returnToMainMenu();
      }
      else if (buttonIndex == 1)
      {
        m_window.close();
      }
      break;

    case GlobalState::Playing:
      break;
  }
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

  if (m_state != GlobalState::Playing) return;

  m_worldTimeSeconds += dtSafe;

  ecs::InputSystem::update(m_registry, m_config, dtSafe, mouseDx);
  ecs::PathfindingSystem::update(m_registry, m_tilemap, dtSafe);
  ecs::AnimationSystem::update(m_registry, dtSafe);
  ecs::PhysicsSystem::update(m_registry, dtSafe, m_tilemap);
  ecs::RayCasting::rayCast(m_registry, m_config, m_player);
  ecs::WeaponSystem::update(m_registry, m_config, m_tilemap, m_player, dtSafe);
  ecs::ProjectileSystem::update(m_registry, m_config, m_tilemap, dtSafe);

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
  if (!m_uiFontLoaded) return;
  if (m_player == ecs::INVALID_ENTITY) return;

  const auto* hp = m_registry.getComponent<ecs::HealthComponent>(m_player);
  if (!hp) return;

  sf::Text text;
  text.setFont(m_uiFont);
  text.setCharacterSize(28);
  text.setFillColor(sf::Color::White);

  const int cur = static_cast<int>(std::round(hp->current));
  const int mx  = static_cast<int>(std::round(hp->max));

  text.setString(toSfStringUtf8("HP: " + std::to_string(cur) + "/" + std::to_string(mx)));
  const auto      bounds  = text.getLocalBounds();
  constexpr float marginX = 18.f;
  constexpr float marginY = 14.f;
  const float     y       = static_cast<float>(m_window.getSize().y) - (bounds.top + bounds.height) - marginY;
  text.setPosition(marginX, y);
  m_window.draw(text);
}


void Game::drawWeaponView()
{
  if (m_player == ecs::INVALID_ENTITY) return;
  if (!m_registry.isAlive(m_player)) return;

  const auto* inv = m_registry.getComponent<ecs::WeaponInventoryComponent>(m_player);
  if (!inv) return;
  if (inv->slots.empty()) return;

  const int activeIndex = std::clamp(inv->activeIndex, 0, static_cast<int>(inv->slots.size()) - 1);
  const auto& slot = inv->slots[static_cast<std::size_t>(activeIndex)];
  if (!slot.weapon) return;

  std::string texId;

  if (slot.firing)
  {
    const auto& fireFrames = slot.weapon->viewFireFrames();
    if (!fireFrames.empty())
    {
      const std::size_t idx = std::min(slot.animFrame, fireFrames.size() - 1u);
      texId = fireFrames[idx];
    }
  }

  if (texId.empty())
  {
    const auto& idle = slot.weapon->viewIdleFrames();
    texId = (!idle.empty()) ? idle.front() : std::string{};
  }

  if (texId.empty()) return;

  const sf::Texture* tex = m_textureManager.get(texId);
  if (!tex) return;

  sf::Sprite spr;
  spr.setTexture(*tex);

  const auto ws = m_window.getSize();
  const auto tsz = tex->getSize();
  if (tsz.x == 0u || tsz.y == 0u) return;

  const float targetHeight = static_cast<float>(ws.y) * 0.42f;
  const float scale = targetHeight / static_cast<float>(tsz.y);
  spr.setScale(scale, scale);

  const float w = static_cast<float>(tsz.x) * scale;
  const float h = static_cast<float>(tsz.y) * scale;

  const float x = (static_cast<float>(ws.x) - w) * 0.5f;
  const float y = static_cast<float>(ws.y) - h;

  spr.setPosition(x, y);

  m_window.draw(spr);
}

void Game::drawMenu(const std::string& title, const std::vector<UIButton>& buttons, const bool darkenBackground)
{
  if (darkenBackground)
  {
    sf::RectangleShape overlay;
    overlay.setSize({static_cast<float>(m_window.getSize().x), static_cast<float>(m_window.getSize().y)});
    overlay.setPosition(0.f, 0.f);
    overlay.setFillColor(sf::Color(0, 0, 0, 140));
    m_window.draw(overlay);
  }

  drawTitle(m_window, m_uiFont, title, m_uiFontLoaded, 160.f);

  for (const auto& b : buttons)
  {
    drawButton(m_window, m_uiFont, b, m_uiFontLoaded);
  }
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
