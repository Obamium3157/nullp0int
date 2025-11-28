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

    const float tileSize = map->tileSize;
    const sf::Vector2f playerWorldPos = posComp->position;
    const sf::Vector2f playerTilePos{ playerWorldPos.x / tileSize, playerWorldPos.y / tileSize };

    const float viewDirectionAngle = radiansFromDegrees(rotComp->angle);

    if (!registry.hasComponent<RayCastResultComponent>(player))
    {
        registry.addComponent<RayCastResultComponent>(player, RayCastResultComponent{});
    }

    auto* result = registry.getComponent<RayCastResultComponent>(player);
    result->hits.clear();
    result->hits.reserve(amount_of_rays);

    float rayAngle = viewDirectionAngle - half_fov + RAY_ANGLE_OFFSET;

    for (unsigned r = 0; r < amount_of_rays; ++r)
    {
        const float sin_a = std::sin(rayAngle);
        const float cos_a = std::cos(rayAngle);

        float nearestHorizontalDist = std::numeric_limits<float>::infinity();
        float horizontalHitWorldX = 0.f, horizontalHitWorldY = 0.f;

        if (std::abs(sin_a) > BIG_EPSILON)
        {
            float y, dy;
            if (sin_a > 0.f) { y = std::floor(playerTilePos.y) + 1.0f;        dy = 1.0f;  }
            else             { y = std::floor(playerTilePos.y) - BIG_EPSILON; dy = -1.0f; }

            float distToNextHorizontal = (y - playerTilePos.y) / sin_a;
            float x_h = playerTilePos.x + distToNextHorizontal * cos_a;
            const float horizontalDistStep = dy / sin_a;
            const float horizontalXStep = horizontalDistStep * cos_a;

            for (unsigned i = 0; i < max_depth; ++i)
            {
                const int tx = static_cast<int>(std::floor(x_h));
                const int ty = static_cast<int>(std::floor(y));
                if (!insideMapIs(map, tx, ty) || map->isWall(tx, ty))
                {
                    nearestHorizontalDist = distToNextHorizontal;
                    horizontalHitWorldX = x_h;
                    horizontalHitWorldY = y;
                    break;
                }
                x_h += horizontalXStep;
                y += dy;
                distToNextHorizontal += horizontalDistStep;
            }
        }

        float nearestVerticalDist = std::numeric_limits<float>::infinity();
        float verticalHitWorldX = 0.f, verticalHitWorldY = 0.f;

        if (std::abs(cos_a) > BIG_EPSILON)
        {
            float x, dx;
            if (cos_a > 0.f) { x = std::floor(playerTilePos.x) + 1.0f;        dx = 1.0f;  }
            else             { x = std::floor(playerTilePos.x) - BIG_EPSILON; dx = -1.0f; }

            float distToNextVertical = (x - playerTilePos.x) / cos_a;
            float y_v = playerTilePos.y + distToNextVertical * sin_a;
            const float verticalDistStep = dx / cos_a;
            const float verticalYStep = verticalDistStep * sin_a;

            for (unsigned i = 0; i < max_depth; ++i)
            {
                const int tx = static_cast<int>(std::floor(x));
                const int ty = static_cast<int>(std::floor(y_v));
                if (!insideMapIs(map, tx, ty) || map->isWall(tx, ty))
                {
                    nearestVerticalDist = distToNextVertical;
                    verticalHitWorldX = x;
                    verticalHitWorldY = y_v;
                    break;
                }
                x += dx;
                y_v += verticalYStep;
                distToNextVertical += verticalDistStep;
            }
        }

        float depthTiles;
        float chosenHitTileX;
        float chosenHitTileY;
        int chosenHitTileIndexX, chosenHitTileIndexY;
        bool chosenVertical;

        if (nearestHorizontalDist < nearestVerticalDist)
        {
            chosenVertical = false;
            depthTiles = nearestHorizontalDist;
            chosenHitTileX = horizontalHitWorldX;
            chosenHitTileY = horizontalHitWorldY;
            chosenHitTileIndexX = static_cast<int>(std::floor(horizontalHitWorldX));
            chosenHitTileIndexY = static_cast<int>(std::floor(horizontalHitWorldY));
        }
        else
        {
            chosenVertical = true;
            depthTiles = nearestVerticalDist;
            chosenHitTileX = verticalHitWorldX;
            chosenHitTileY = verticalHitWorldY;
            chosenHitTileIndexX = static_cast<int>(std::floor(verticalHitWorldX));
            chosenHitTileIndexY = static_cast<int>(std::floor(verticalHitWorldY));
        }

        const float depthWorld = depthTiles * tileSize;
        const sf::Vector2f hitPointWorld{ chosenHitTileX * tileSize, chosenHitTileY * tileSize };

        RayHit hit;
        hit.hitPointWorld = hitPointWorld;
        hit.distance = depthWorld;
        hit.tileX = chosenHitTileIndexX;
        hit.tileY = chosenHitTileIndexY;
        hit.vertical = chosenVertical;

        result->hits.push_back(hit);

        rayAngle += delta_angle;
    }
}
