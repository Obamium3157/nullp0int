//
// Created by obamium3157 on 27.12.2025.
//

#ifndef NULLP0INT_GAMEUI_H
#define NULLP0INT_GAMEUI_H
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/String.hpp>

#include "Game.h"

[[nodiscard]] bool loadUiFont(sf::Font& font);
[[nodiscard]] sf::String toSfStringUtf8(std::string_view s);
[[nodiscard]] sf::FloatRect centeredRect(sf::Vector2u winSize, float y, float w, float h);
void setMouseCaptured(sf::RenderWindow& window, bool captured);
[[nodiscard]] sf::Vector2i windowCenterPx(const sf::RenderWindow& window);
[[nodiscard]] float consumeMouseDeltaX(const sf::RenderWindow& window, bool captureNow);
void drawButton(sf::RenderWindow& window, const sf::Font& font, const UIButton& b, bool fontLoaded);
void drawTitle(sf::RenderWindow& window, const sf::Font& font, const std::string& title, bool fontLoaded, float y);

#endif //NULLP0INT_GAMEUI_H