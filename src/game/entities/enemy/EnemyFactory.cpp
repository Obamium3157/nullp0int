//
// Created by obamium3157 on 06.12.2025.
//

#include "EnemyFactory.h"

#include "../../../ecs/Components.h"

namespace
{
  [[nodiscard]] bool enemyClassFromMarker(const char c, ecs::EnemyClass& outCls)
  {
    switch (c)
    {
      case MELEE_ENEMY_SPAWN_MARKER:
        outCls = ecs::EnemyClass::MELEE;
        return true;
      case RANGE_ENEMY_SPAWN_MARKER:
        outCls = ecs::EnemyClass::RANGE;
        return true;
      case SUPPORT_ENEMY_SPAWN_MARKER:
        outCls = ecs::EnemyClass::SUPPORT;
        return true;
      default:
        return false;
    }
  }

  [[nodiscard]] sf::Vector2f tileCenterWorld(const int tx, const int ty, const float tileSize)
  {
    return sf::Vector2f{
      (static_cast<float>(tx) + 0.5f) * tileSize,
      (static_cast<float>(ty) + 0.5f) * tileSize
    };
  }
}

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

void spawnEnemiesFromMap(ecs::Registry &registry, const ecs::Entity tilemapEntity, const Configuration &config)
{
  auto* map = registry.getComponent<ecs::TilemapComponent>(tilemapEntity);
  if (!map) return;

  const float enemyRadius = config.player_radius;
  const float enemySpeed  = config.player_speed * 0.5f;

  const int h = static_cast<int>(map->tiles.size());
  for (int y = 0; y < h; ++y)
  {
    auto& row = map->tiles[static_cast<std::size_t>(y)];
    const int w = static_cast<int>(row.size());

    for (int x = 0; x < w; ++x)
    {
      ecs::EnemyClass cls{};
      if (!enemyClassFromMarker(row[static_cast<std::size_t>(x)], cls)) continue;

      const sf::Vector2f spawnPos = tileCenterWorld(x, y, map->tileSize);
      (void)initEnemy(registry, cls, spawnPos, enemyRadius, enemySpeed);

      row[static_cast<std::size_t>(x)] = FLOOR_MARKER;
    }
  }
}
