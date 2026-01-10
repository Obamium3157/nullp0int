//
// Created by obamium3157 on 13.11.2025.
//

#ifndef NULLP0INT_GAME_H
#define NULLP0INT_GAME_H

#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include "Hud.h"
#include "../constants.h"
#include "../configuration/Configuration.h"
#include "../ecs/Registry.h"
#include "../ecs/systems/render/TextureManager.h"

struct UIButton
{
  sf::FloatRect rect;
  std::string label;
};

class Game
{
public:
  explicit Game(unsigned windowW = SCREEN_WIDTH,
                unsigned windowH = SCREEN_HEIGHT,
                const std::string& title = "NULLP0INT",
                unsigned antialiasing = 0);

  void run();

private:
  enum class GlobalState
  {
    MainMenu,
    MapSelect,
    Playing,
    Paused,
    UpgradeMenu,
    WinMenu,
    DeathMenu,
  };

  enum class MapChoice
  {
    TestMap,
    Procedural,
  };

  sf::RenderWindow m_window;
  ecs::Registry m_registry;
  TextureManager m_textureManager;

  Configuration m_config;

  ecs::Entity m_player = ecs::INVALID_ENTITY;
  ecs::Entity m_tilemap = ecs::INVALID_ENTITY;

  bool m_campaignActive = false;
  int m_campaignLevelIndex = 0;
  std::vector<std::string> m_campaignMapPaths;

  GlobalState m_state = GlobalState::MainMenu;
  float m_worldTimeSeconds = 0.f;

  sf::Font m_uiFont;
  bool m_uiFontLoaded = false;

  Hud m_hud;

  void init_textures();
  void init_player();
  void init_tilemap(MapChoice choice);
  void init();

  void startNewGame(MapChoice choice);
  void returnToMainMenu();

  void generateProceduralCampaign(uint32_t seed);
  void loadCampaignLevel(int index);
  bool tryOpenExit();

  [[nodiscard]] std::vector<UIButton> buildButtonsForState(GlobalState state) const;
  void drawMenu(const std::string& title, const std::vector<UIButton>& buttons, bool darkenBackground);
  void drawHud();
  void drawWeaponView();
  void setState(GlobalState next);

  void onMenuButtonPressed(std::size_t buttonIndex);

  void handleEvents();
  void update(float dt);
  void render();
};

#endif //NULLP0INT_GAME_H
