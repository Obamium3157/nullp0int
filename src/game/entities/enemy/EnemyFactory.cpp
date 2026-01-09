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

  [[nodiscard]] std::uint32_t seedFromPos(const sf::Vector2f p, const ecs::EnemyClass cls)
  {
    const auto x = static_cast<std::uint32_t>(static_cast<int>(p.x) * 73856093);
    const auto y = static_cast<std::uint32_t>(static_cast<int>(p.y) * 19349663);
    const std::uint32_t c = static_cast<std::uint32_t>(cls) * 83492791u;
    std::uint32_t s = x ^ y ^ c ^ 0x9E3779B9u;
    if (s == 0u) s = 0xA3C59AC3u;
    return s;
  }
}

ecs::Entity initEnemy(ecs::Registry& registry,
                     const ecs::EnemyClass cls,
                     const sf::Vector2f initialPos,
                     const float radius)
{
  const ecs::Entity enemy = registry.createEntity();
  registry.addComponent<ecs::PositionComponent>(enemy, ecs::PositionComponent{initialPos});
  registry.addComponent<ecs::RadiusComponent>(enemy, ecs::RadiusComponent{radius});
  registry.addComponent<ecs::VelocityComponent>(enemy, ecs::VelocityComponent{});
  registry.addComponent<ecs::EnemyTag>(enemy, ecs::EnemyTag{});

  float speed = 0.f;
  float maxHp = 40.f;

  ecs::EnemyComponent ec;
  ec.cls = cls;
  ec.spriteScale = 1.0f;
  ec.heightShift = 0.27f;
  ec.hasSeenPlayer = false;
  ec.state = ecs::EnemyState::PASSIVE;

  ec.meleeAttackRangeTiles = 1.15f;
  ec.meleeAttackDamage = 25.f;

  ec.rangedPreferredRangeTiles = 10.f;
  ec.rangedRangeToleranceTiles = 2.f;
  ec.rangedAttackRangeTiles = 18.f;
  ec.rangedAttackDamage = 6.f;

  ec.attackCooldownSeconds = (cls == ecs::EnemyClass::MELEE) ? 0.f : 0.25f;
  ec.cooldownRemainingSeconds = 0.f;
  ec.rngState = seedFromPos(initialPos, cls);

  switch (cls)
  {
    case ecs::EnemyClass::MELEE:
      ec.textureId = "melee_walk_1";
      ec.walkFrames = {"melee_walk_1", "melee_walk_2"};
      ec.attackFrames = {"melee_attack_1", "melee_attack_2", "melee_attack_3"};
      ec.walkFrameTime = 0.3f;
      ec.attackFrameTime = 0.13f;

      speed = 375.f;
      maxHp = 75.f;
      break;
    case ecs::EnemyClass::RANGE:
      ec.textureId = "range_walk_1";
      ec.walkFrames = {"range_walk_1", "range_walk_2"};
      ec.walkFramesLeft = {"range_walk_left_1", "range_walk_left_2"};
      ec.walkFramesRight = {"range_walk_right_1", "range_walk_right_2"};
      ec.walkFramesBack = {"range_walk_back_1", "range_walk_back_2"};
      ec.attackFrames = {"range_attack_1", "range_attack_2"};
      ec.walkFrameTime = 0.3f;
      ec.attackFrameTime = 0.26f;

      speed = 200.f;
      maxHp = 35.f;
      break;
    case ecs::EnemyClass::SUPPORT:
      ec.textureId = "support_walk_1";
      ec.walkFrames = {"support_walk_1", "support_walk_2", "support_walk_3", "support_walk_4"};
      ec.walkFramesLeft = {"support_walk_left_1", "support_walk_left_2", "support_walk_left_3"};
      ec.walkFramesRight = {"support_walk_right_1", "support_walk_right_2", "support_walk_right_3"};
      ec.walkFramesBack = {"support_walk_back_1", "support_walk_back_2"};
      ec.attackFrames = {"support_attack_1", "support_attack_2"};
      ec.walkFrameTime = 0.3f;
      ec.attackFrameTime = 0.16f;

      speed = 150.f;
      maxHp = 60.f;
      break;
  }

  registry.addComponent<ecs::SpeedComponent>(enemy, ecs::SpeedComponent{speed});
  registry.addComponent<ecs::HealthComponent>(enemy, ecs::HealthComponent{maxHp, maxHp});

  registry.addComponent<ecs::EnemyComponent>(enemy, ec);

  ecs::SpriteComponent sc;
  sc.textureFrames = ec.walkFrames;
  sc.frameTime = ec.walkFrameTime;
  sc.loop = true;
  sc.playing = false;
  sc.currentFrame = 0;
  sc.frameAccumulator = 0.f;
  sc.textureId = ec.textureId;
  registry.addComponent<ecs::SpriteComponent>(enemy, sc);

  return enemy;
}

void spawnEnemiesFromMap(ecs::Registry &registry, const ecs::Entity tilemapEntity, const Configuration &config)
{
  auto* map = registry.getComponent<ecs::TilemapComponent>(tilemapEntity);
  if (!map) return;

  const float enemyRadius = config.player_radius;

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
      (void)initEnemy(registry, cls, spawnPos, enemyRadius);

      row[static_cast<std::size_t>(x)] = FLOOR_MARKER;
    }
  }
}
