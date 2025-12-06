//
// Created by obamium3157 on 06.12.2025.
//

#include "EnemyFactory.h"

#include "../../../ecs/Components.h"

ecs::Entity initEnemy(ecs::Registry &registry, const ecs::EnemyClass cls, const sf::Vector2f initialPos, const float radius, const float speed)
{
  const ecs::Entity enemy = registry.createEntity();
  registry.addComponent<ecs::PositionComponent>(enemy, ecs::PositionComponent{initialPos});
  registry.addComponent<ecs::RadiusComponent>(enemy, ecs::RadiusComponent{radius});
  registry.addComponent<ecs::VelocityComponent>(enemy, ecs::VelocityComponent{});
  registry.addComponent<ecs::SpeedComponent>(enemy, ecs::SpeedComponent{speed});

  registry.addComponent<ecs::EnemyTag>(enemy, ecs::EnemyTag{});

  std::string textureType;
  switch (cls)
  {
    case ecs::EnemyClass::MELEE:
      textureType = "melee";
      break;
    case ecs::EnemyClass::RANGE:
      textureType = "range";
      break;
    case ecs::EnemyClass::SUPPORT:
      textureType = "support";
      break;
  }

  registry.addComponent<ecs::EnemyComponent>(enemy, ecs::EnemyComponent{
        cls,
        textureType + "_enemy",
        1.0f,
        0.27f
      });

  if (cls == ecs::EnemyClass::MELEE)
  {
    ecs::SpriteComponent sc;
    sc.textureFrames = { "melee_enemy_1", "melee_enemy_2" };
    sc.frameTime = 0.09f;
    sc.playing = true;
    sc.loop = true;
    sc.currentFrame = 0;
    sc.frameAccumulator = 0.f;
    sc.textureId = sc.textureFrames.front();

    registry.addComponent<ecs::SpriteComponent>(enemy, sc);
  }

  return enemy;
}
