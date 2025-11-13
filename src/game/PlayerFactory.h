//
// Created by obamium3157 on 13.11.2025.
//

#ifndef NULLP0INT_PLAYERFACTORY_H
#define NULLP0INT_PLAYERFACTORY_H

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

#include "../ecs/Registry.h"

ecs::Entity initPlayer(ecs::Registry &registry, sf::Vector2f initialPos, sf::Color color);

#endif //NULLP0INT_PLAYERFACTORY_H