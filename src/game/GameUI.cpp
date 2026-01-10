//
// Created by obamium3157 on 27.12.2025.
//

#include <filesystem>

#include "GameUI.h"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Mouse.hpp>

#include "Game.h"

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