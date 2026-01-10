//
// Created by obamium3157 on 27.12.2025.
//

#include <algorithm>
#include <random>

#include "Game.h"
#include "GameUI.h"
#include "../ecs/Components.h"
#include "entities/enemy/EnemyFactory.h"

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
