//
// Created by obamium3157 on 27.12.2025.
//

#include <algorithm>
#include <random>

#include "Game.h"
#include "GameUI.h"
#include "../ecs/Components.h"
#include "../ecs/systems/map/MapLoaderSystem.h"
#include "../ecs/systems/map_generation/MapGenerationSystem.h"
#include "entities/enemy/EnemyFactory.h"

namespace
{
  constexpr int kCampaignLevels = 3;
  constexpr int kFirstLevelSide = 100;
  constexpr int kSideStep = 50;

  int campaignSideForLevel(const int idx)
  {
    return kFirstLevelSide + idx * kSideStep;
  }

  int campaignRoomsForLevel(const int idx)
  {
    return 50 + idx * 25;
  }
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
  m_campaignActive = false;
  m_campaignLevelIndex = 0;
  m_campaignMapPaths.clear();
  setState(GlobalState::MainMenu);
}

void Game::startNewGame(const MapChoice choice)
{
  std::random_device rd;
  const auto seed = rd();

  if (choice == MapChoice::Procedural)
  {
    m_campaignActive = true;
    m_campaignLevelIndex = 0;
    generateProceduralCampaign(seed);
    loadCampaignLevel(0);
    return;
  }

  m_campaignActive = false;
  m_campaignLevelIndex = 0;
  m_campaignMapPaths.clear();

  m_registry = ecs::Registry{};
  m_player = ecs::INVALID_ENTITY;
  m_tilemap = ecs::INVALID_ENTITY;
  m_worldTimeSeconds = 0.f;

  init_tilemap(seed, choice);
  init_textures();
  init_player();
  spawnEnemiesFromMap(m_registry, m_tilemap, m_config);

  setState(GlobalState::Playing);
}

void Game::generateProceduralCampaign(const uint32_t seed)
{
  m_campaignMapPaths.clear();
  m_campaignMapPaths.reserve(kCampaignLevels);

  for (int i = 0; i < kCampaignLevels; ++i)
  {
    const int side = campaignSideForLevel(i);
    const int rooms = campaignRoomsForLevel(i);
    MapGenerationSystem mgs{side, side, seed + static_cast<uint32_t>(i) * 1337u};
    const std::string filename = mgs.generateLevel(rooms);
    m_campaignMapPaths.push_back("resources/maps/generated/" + filename);
  }
}

void Game::loadCampaignLevel(const int index)
{
  if (index < 0 || index >= static_cast<int>(m_campaignMapPaths.size()))
  {
    returnToMainMenu();
    return;
  }

  m_campaignLevelIndex = index;

  m_registry = ecs::Registry{};
  m_player = ecs::INVALID_ENTITY;
  m_tilemap = ecs::INVALID_ENTITY;
  m_worldTimeSeconds = 0.f;

  m_tilemap = ecs::MapLoaderSystem::load(m_registry, m_config, m_campaignMapPaths[static_cast<std::size_t>(index)]);
  init_textures();
  init_player();
  spawnEnemiesFromMap(m_registry, m_tilemap, m_config);

  setState(GlobalState::Playing);
}

bool Game::tryOpenExit()
{
  if (!m_campaignActive) return false;
  if (m_state != GlobalState::Playing) return false;
  if (m_player == ecs::INVALID_ENTITY || !m_registry.isAlive(m_player)) return false;
  if (m_tilemap == ecs::INVALID_ENTITY) return false;

  const auto* tm = m_registry.getComponent<ecs::TilemapComponent>(m_tilemap);
  const auto* pos = m_registry.getComponent<ecs::PositionComponent>(m_player);
  if (!tm || !pos) return false;

  sf::Vector2i endTile{-1, -1};
  for (unsigned ty = 0; ty < tm->height; ++ty)
  {
    for (unsigned tx = 0; tx < tm->width; ++tx)
    {
      if (tm->tiles[ty][tx] == END_MARKER)
      {
        endTile = {static_cast<int>(tx), static_cast<int>(ty)};
        break;
      }
    }
    if (endTile.x >= 0) break;
  }

  if (endTile.x < 0) return false;

  const sf::Vector2i playerTile = tm->worldToTile(pos->position);
  const int dx = std::abs(playerTile.x - endTile.x);
  const int dy = std::abs(playerTile.y - endTile.y);
  if (std::max(dx, dy) > 1) return false;

  if (m_campaignLevelIndex >= kCampaignLevels - 1)
  {
    setState(GlobalState::WinMenu);
  }
  else
  {
    setState(GlobalState::UpgradeMenu);
  }
  return true;
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

    case GlobalState::UpgradeMenu:
      push(340.f, "Улучшение");
      push(340.f + (btnH + spacing), "Улучшение");
      push(340.f + 2.f * (btnH + spacing), "Улучшение");
      break;

    case GlobalState::WinMenu:
      push(static_cast<float>(ws.y) - 220.f, "В главное меню");
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

    case GlobalState::UpgradeMenu:
      if (m_campaignActive)
      {
        loadCampaignLevel(m_campaignLevelIndex + 1);
      }
      else
      {
        setState(GlobalState::Playing);
      }
      break;

    case GlobalState::WinMenu:
      if (buttonIndex == 0)
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

  if (const auto* parry = m_registry.getComponent<ecs::ParryComponent>(m_player);
    parry && parry->parrying)
  {
    static const std::string kFrames[3] = {"parry1", "parry2", "parry3"};
    const std::size_t idx = std::min<std::size_t>(parry->animFrame, 2u);
    texId = kFrames[idx];
  }
  if (texId.empty() && slot.firing)
  {
    if (const auto& fireFrames = slot.weapon->viewFireFrames(); !fireFrames.empty())
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
