//
// Created by obamium3157 on 14.11.2025.
//

#include "RayCasting.h"

#include <cmath>
#include <limits>

#include "../../Components.h"
#include "../../Registry.h"
#include "../../../constants.h"
#include "../../../math/mathUtils.h"
#include "../map/MapLoaderSystem.h"

ecs::Entity ecs::RayCasting::findTilemapEntity(const Registry &registry)
{
    for (const auto &e : registry.entities())
    {
        if (registry.hasComponent<TilemapComponent>(e))
        {
            return e;
        }
    }
    return INVALID_ENTITY;
}

void ecs::RayCasting::rayCast(Registry &registry, Configuration config, const Entity &player)
{
    const auto* posComp = registry.getComponent<PositionComponent>(player);
    const auto* rotComp = registry.getComponent<RotationComponent>(player);
    if (!posComp || !rotComp) return;

    const Entity mapEntity = findTilemapEntity(registry);
    if (mapEntity == INVALID_ENTITY) return;
    const auto* map = registry.getComponent<TilemapComponent>(mapEntity);
    if (!map) return;

    const auto amount_of_rays = static_cast<unsigned>(config.resolution_option);
    const auto fov = config.fov;
    const auto half_fov = fov / 2.f;
    const auto max_depth = config.render_distance;
    const auto delta_angle = fov / static_cast<float>(amount_of_rays);

    const auto tileSize = static_cast<double>(map->tileSize);
    const sf::Vector2f playerWorldPos = posComp->position;
    const double playerTileX = static_cast<double>(playerWorldPos.x) / tileSize;
    const double playerTileY = static_cast<double>(playerWorldPos.y) / tileSize;

    const auto viewDirectionAngle = static_cast<double>(radiansFromDegrees(rotComp->angle));

    if (!registry.hasComponent<RayCastResultComponent>(player))
    {
        registry.addComponent<RayCastResultComponent>(player, RayCastResultComponent{});
    }

    auto* result = registry.getComponent<RayCastResultComponent>(player);
    result->hits.clear();
    result->hits.reserve(amount_of_rays);

    double rayAngle = (viewDirectionAngle - half_fov) + static_cast<double>(RAY_ANGLE_OFFSET);

    for (unsigned r = 0; r < amount_of_rays; ++r)
    {
        const double sin_a = std::sin(rayAngle);
        const double cos_a = std::cos(rayAngle);

        double nearestHorizontalDist = std::numeric_limits<double>::infinity();
        double horizontalHitTileX = 0.0, horizontalHitTileY = 0.0;

        if (std::abs(sin_a) > static_cast<double>(BIG_EPSILON))
        {
            double y, dy;
            if (sin_a > 0.0) { y = std::floor(playerTileY) + 1.0;         dy = 1.0;  }
            else             { y = std::floor(playerTileY) - BIG_EPSILON; dy = -1.0; }

            double distToNextHorizontal = (y - playerTileY) / sin_a;
            double x_h = playerTileX + distToNextHorizontal * cos_a;
            const double horizontalDistStep = dy / sin_a;
            const double horizontalXStep = horizontalDistStep * cos_a;

            for (unsigned i = 0; i < max_depth; ++i)
            {
                const int tx = static_cast<int>(std::floor(x_h));
                const int ty = static_cast<int>(std::floor(y));
                if (map->isWall(tx, ty))
                {
                    nearestHorizontalDist = distToNextHorizontal;
                    horizontalHitTileX = x_h;
                    horizontalHitTileY = y;
                    break;
                }
                x_h += horizontalXStep;
                y += dy;
                distToNextHorizontal += horizontalDistStep;
            }
        }

        double nearestVerticalDist = std::numeric_limits<double>::infinity();
        double verticalHitTileX = 0.0, verticalHitTileY = 0.0;

        if (std::abs(cos_a) > static_cast<double>(BIG_EPSILON))
        {
            double x, dx;
            if (cos_a > 0.0) { x = std::floor(playerTileX) + 1.0;         dx = 1.0;  }
            else             { x = std::floor(playerTileX) - BIG_EPSILON; dx = -1.0; }

            double distToNextVertical = (x - playerTileX) / cos_a;
            double y_v = playerTileY + distToNextVertical * sin_a;
            const double verticalDistStep = dx / cos_a;
            const double verticalYStep = verticalDistStep * sin_a;

            for (unsigned i = 0; i < max_depth; ++i)
            {
                const int tx = static_cast<int>(std::floor(x));
                const int ty = static_cast<int>(std::floor(y_v));
                if (map->isWall(tx, ty))
                {
                    nearestVerticalDist = distToNextVertical;
                    verticalHitTileX = x;
                    verticalHitTileY = y_v;
                    break;
                }
                x += dx;
                y_v += verticalYStep;
                distToNextVertical += verticalDistStep;
            }
        }

        double depthTiles;
        double chosenHitTileX;
        double chosenHitTileY;
        int chosenHitTileIndexX, chosenHitTileIndexY;
        bool chosenVertical;

        if (nearestHorizontalDist + EPSILON_DIST < nearestVerticalDist)
        {
            chosenVertical = false;
            depthTiles = nearestHorizontalDist;
            chosenHitTileX = horizontalHitTileX;
            chosenHitTileY = horizontalHitTileY;
            chosenHitTileIndexX = static_cast<int>(std::floor(horizontalHitTileX));
            chosenHitTileIndexY = static_cast<int>(std::floor(horizontalHitTileY));
        }
        else
        {
            chosenVertical = true;
            depthTiles = nearestVerticalDist;
            chosenHitTileX = verticalHitTileX;
            chosenHitTileY = verticalHitTileY;
            chosenHitTileIndexX = static_cast<int>(std::floor(verticalHitTileX));
            chosenHitTileIndexY = static_cast<int>(std::floor(verticalHitTileY));
        }
        const double depthWorld = depthTiles * tileSize;
        const sf::Vector2f hitPointWorld{ static_cast<float>(chosenHitTileX * tileSize), static_cast<float>(chosenHitTileY * tileSize) };

        RayHit hit;
        hit.hitPointWorld = hitPointWorld;
        hit.distance = static_cast<float>(depthWorld);
        hit.tileX = chosenHitTileIndexX;
        hit.tileY = chosenHitTileIndexY;
        hit.vertical = chosenVertical;
        hit.rayAngle = static_cast<float>(rayAngle);

        result->hits.push_back(hit);

        rayAngle += static_cast<double>(delta_angle);
    }
}
