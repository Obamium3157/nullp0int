#include "game/Game.h"
#include "SFML/Graphics.hpp"

int main()
{
  constexpr unsigned WINDOW_WIDTH = 1920;
  constexpr unsigned WINDOW_HEIGHT = 1080;

  sf::ContextSettings settings;
  settings.antialiasingLevel = 8;

  Game game(WINDOW_WIDTH, WINDOW_HEIGHT, "NULLP0INT", settings);
  game.run();

  return 0;
}