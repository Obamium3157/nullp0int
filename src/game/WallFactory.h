//
// Created by obamium3157 on 13.11.2025.
//

#ifndef NULLP0INT_WALLFACTORY_H
#define NULLP0INT_WALLFACTORY_H
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

#include "../ecs/Entity.h"
#include "../ecs/Registry.h"

ecs::Entity initWall(ecs::Registry &registry, const sf::Vector2f pos, const sf::Vector2f size, const sf::Color fillColor);

#endif //NULLP0INT_WALLFACTORY_H
