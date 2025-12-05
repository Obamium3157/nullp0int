//
// Created by obamium3157 on 06.12.2025.
//

#ifndef NULLP0INT_ENEMYFACTORY_H
#define NULLP0INT_ENEMYFACTORY_H
#include <SFML/System/Vector2.hpp>

#include "../../../ecs/Components.h"
#include "../../../ecs/Entity.h"
#include "../../../ecs/Registry.h"


ecs::Entity initEnemy(ecs::Registry &registry, ecs::EnemyClass cls, sf::Vector2f initialPos, float radius, float speed);


#endif //NULLP0INT_ENEMYFACTORY_H
