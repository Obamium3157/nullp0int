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


static bool inside_map_is(const ecs::TilemapComponent* map, const int x, const int y)
{
    if (!map) return false;
    return x >= 0 && y >= 0 && x < static_cast<int>(map->width) && y < static_cast<int>(map->height);
}

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

void ecs::RayCasting::rayCast(Registry &registry, Entity &player)
{
    auto* posComp = registry.getComponent<PositionComponent>(player);
    auto* rotComp = registry.getComponent<RotationComponent>(player);
    if (!posComp || !rotComp) return;

    const Entity mapEntity = findTilemapEntity(registry);
    if (mapEntity == INVALID_ENTITY) return;
    const auto* map = registry.getComponent<TilemapComponent>(mapEntity);
    if (!map) return;

    const float tileSize = map->tileScale;
    const sf::Vector2f playerWorldPos = posComp->position;
    const sf::Vector2f playerTilePos{ playerWorldPos.x / tileSize, playerWorldPos.y / tileSize };

    const float ang = radiansFromDegrees(rotComp->angle);

    if (!registry.hasComponent<RayCastResultComponent>(player))
    {
        registry.addComponent<RayCastResultComponent>(player, RayCastResultComponent{});
    }

    auto* result = registry.getComponent<RayCastResultComponent>(player);
    result->hits.clear();
    result->hits.reserve(AMOUNT_OF_RAYS);


    float rayAngle = ang - HALF_FOV + RAY_ANGLE_OFFSET;

    for (unsigned r = 0; r < AMOUNT_OF_RAYS; ++r)
    {
        const float sin_a = std::sin(rayAngle);
        const float cos_a = std::cos(rayAngle);

        float depthH = std::numeric_limits<float>::infinity();
        float xH = 0.f, yH = 0.f, texH = 0.f;


        if (std::abs(sin_a) > BIG_EPSILON)
        {
            float y, dy;
            if (sin_a > 0.f) { y = std::floor(playerTilePos.y) + 1.0f;        dy = 1.0f;  }
            else             { y = std::floor(playerTilePos.y) - BIG_EPSILON; dy = -1.0f; }

            float d_h = (y - playerTilePos.y) / sin_a;
            float x_h = playerTilePos.x + d_h * cos_a;
            const float ddh = dy / sin_a;
            const float dxh = ddh * cos_a;

            for (unsigned i = 0; i < MAX_DEPTH; ++i)
            {
                const int tx = static_cast<int>(std::floor(x_h));
                const int ty = static_cast<int>(std::floor(y));
                if (!inside_map_is(map, tx, ty) || map->isWall(tx, ty))
                {
                    depthH = d_h;
                    xH = x_h;
                    yH = y;
                    texH = xH - std::floor(xH);
                    break;
                }
                x_h += dxh;
                y += dy;
                d_h += ddh;
            }
        }

        float depthV = std::numeric_limits<float>::infinity();
        float xV = 0.f, yV = 0.f, texV = 0.f;

        if (std::abs(cos_a) > BIG_EPSILON)
        {
            float x, dx;
            if (cos_a > 0.f) { x = std::floor(playerTilePos.x) + 1.0f;        dx = 1.0f;  }
            else             { x = std::floor(playerTilePos.x) - BIG_EPSILON; dx = -1.0f; }

            float d_v = (x - playerTilePos.x) / cos_a;
            float y_v = playerTilePos.y + d_v * sin_a;
            const float ddv = dx / cos_a;
            const float dyv = ddv * sin_a;

            for (unsigned i = 0; i < MAX_DEPTH; ++i)
            {
                const int tx = static_cast<int>(std::floor(x));
                const int ty = static_cast<int>(std::floor(y_v));
                if (!inside_map_is(map, tx, ty) || map->isWall(tx, ty))
                {
                    depthV = d_v;
                    xV = x;
                    yV = y_v;
                    texV = yV - std::floor(yV);
                    break;
                }
                x += dx;
                y_v += dyv;
                d_v += ddv;
            }
        }

        float depthTiles;
        float hitXTile;
        float hitYTile;
        int hitTx = -1, hitTy = -1;
        float texOffset = 0.f;

        if (depthH < depthV)
        {
            depthTiles = depthH;
            hitXTile = xH;
            hitYTile = yH;
            hitTx = static_cast<int>(std::floor(xH));
            hitTy = static_cast<int>(std::floor(yH));
            texOffset = texH;
        }
        else
        {
            depthTiles = depthV;
            hitXTile = xV;
            hitYTile = yV;
            hitTx = static_cast<int>(std::floor(xV));
            hitTy = static_cast<int>(std::floor(yV));
            texOffset = texV;
        }

        const float depthWorld = depthTiles * tileSize;
        const sf::Vector2f hitPointWorld{ hitXTile * tileSize, hitYTile * tileSize };

        RayHit hit;
        hit.hitPointWorld = hitPointWorld;
        hit.distance = depthWorld;
        hit.tileX = hitTx;
        hit.tileY = hitTy;
        hit.textureOffset = texOffset;

        result->hits.push_back(hit);

        rayAngle += DELTA_ANGLE;
    }
}
