//
// Created by obamium3157 on 14.11.2025.
//

#include "RenderSystem.h"

#include <algorithm>
#include <cmath>
#include <SFML/Graphics/RectangleShape.hpp>

#include "TextureManager.h"
#include "../../Components.h"
#include "../../Registry.h"
#include "../../../constants.h"
#include "../../../configuration/Configuration.h"
#include "../../../math/mathUtils.h"

using namespace ecs;

void RenderSystem::render(Registry &registry, const Configuration &config, sf::RenderWindow &window, const Entity &m_tilemap, const float globalTime, const TextureManager &textureManager)
{
  const sf::Color ceilingColor(4, 2, 115);
  sf::RectangleShape ceiling(sf::Vector2f(static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT) / 2.f));
  ceiling.setPosition(0.f, 0.f);
  ceiling.setFillColor(ceilingColor);
  window.draw(ceiling);

  if (const auto* tilemapComp = registry.getComponent<TilemapComponent>(m_tilemap); !tilemapComp)
  {
    return;
  }

  Entity playerEntity = INVALID_ENTITY;
  for (const auto& e : registry.entities())
  {
    if (registry.hasComponent<PlayerTag>(e))
    {
      playerEntity = e;
      break;
    }
  }
  if (playerEntity == INVALID_ENTITY) return;

  const auto* posComp = registry.getComponent<PositionComponent>(playerEntity);
  const auto* rotComp = registry.getComponent<RotationComponent>(playerEntity);
  const auto* rayResults = registry.getComponent<RayCastResultComponent>(playerEntity);

  if (!posComp || !rotComp || !rayResults) return;

  renderFloor(registry, config, window, m_tilemap, rotComp->angle, posComp->position, textureManager);

  std::vector<RenderItem> items;
  items.reserve(MAX_ITEMS_TO_RENDER);

  renderWalls(registry, config, m_tilemap, rotComp->angle, *rayResults, globalTime, textureManager, items);
  renderEnemies(registry, config, textureManager, items);
  renderProjectiles(registry, config, textureManager, items);

  std::ranges::sort(
    items, [](const RenderItem& a, const RenderItem& b){
    return a.depth > b.depth;
  });
  for (const auto &it : items)
  {
    window.draw(it.sprite);
  }
}


void RenderSystem::renderFloor(Registry &registry, Configuration config, sf::RenderWindow &window, const Entity& m_tilemap, float playerRotationAngle, const sf::Vector2f playerPos, const TextureManager& textureManager)
{
  auto* tilemapComp = registry.getComponent<TilemapComponent>(m_tilemap);
  if (!tilemapComp) return;

  const std::string floorTexId = tilemapComp->floorTextureId;
  const sf::Texture* floorTex = nullptr;
  if (!floorTexId.empty())
  {
    floorTex = textureManager.get(floorTexId);
  }

  if (!floorTex)
  {
    const sf::Color floorColor(50, 50, 50);
    sf::RectangleShape floorBg(sf::Vector2f(static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT) / 2.f));
    floorBg.setPosition(0.f, static_cast<float>(SCREEN_HEIGHT) / 2.f);
    floorBg.setFillColor(floorColor);
    window.draw(floorBg);
    return;
  }

  const auto fov = config.fov;
  const auto half_fov = fov / 2.f;
  const auto player_eye_height = config.player_eye_height;
  const auto max_linear_attenuation_distance = config.attenuation_distance;
  const auto tile_size = config.tile_size;

  const float screenDist = HALF_SCREEN_WIDTH / std::tan(half_fov);

  const float ang = radiansFromDegrees(playerRotationAngle);
  const float leftAngle = ang - half_fov;
  const float rightAngle = ang + half_fov;

  const sf::Vector2f dirLeft{ std::cos(leftAngle), std::sin(leftAngle) };
  const sf::Vector2f dirRight{ std::cos(rightAngle), std::sin(rightAngle) };

  const float texW = static_cast<float>(floorTex->getSize().x);
  const float texH = static_cast<float>(floorTex->getSize().y);

  sf::RenderStates states;
  states.texture = floorTex;


  constexpr int ROW_START = HALF_SCREEN_HEIGHT;
  constexpr int ROW_END   = SCREEN_HEIGHT - 1;
  constexpr int ROW_STEP  = 1;

  constexpr int rowsCount = ((ROW_END - ROW_START) / ROW_STEP) + 1;

  sf::VertexArray va(sf::Quads);
  va.resize(static_cast<std::size_t>(rowsCount) * 4);

  int vaIndex = 0;
  for (int y = ROW_START; y <= ROW_END; y += ROW_STEP)
  {
    float p = static_cast<float>(y) - HALF_SCREEN_HEIGHT;
    if (std::abs(p) < SMALL_EPSILON)
    {
      p = SMALL_EPSILON;
    }

    float rowDistance = (player_eye_height * screenDist) / p;
    if (!std::isfinite(rowDistance)) continue;
    if (const float maxRowDist = tile_size * max_linear_attenuation_distance * 2.f; rowDistance > maxRowDist) rowDistance = maxRowDist;

    const sf::Vector2f floorPointLeft  = playerPos + dirLeft  * rowDistance;
    const sf::Vector2f floorPointRight = playerPos + dirRight * rowDistance;

    float uLeft  = (floorPointLeft.x  / tile_size) * texW;
    float vLeft  = (floorPointLeft.y  / tile_size) * texH;
    float uRight = (floorPointRight.x / tile_size) * texW;
    float vRight = (floorPointRight.y / tile_size) * texH;

    if (std::abs(uLeft) > texW * SAFE_REPEAT_BEFORE_NORMALIZATION)  uLeft  = std::fmod(uLeft,  texW);
    if (std::abs(uRight) > texW * SAFE_REPEAT_BEFORE_NORMALIZATION) uRight = std::fmod(uRight, texW);
    if (std::abs(vLeft) > texH * SAFE_REPEAT_BEFORE_NORMALIZATION)  vLeft  = std::fmod(vLeft,  texH);
    if (std::abs(vRight) > texH * SAFE_REPEAT_BEFORE_NORMALIZATION) vRight = std::fmod(vRight, texH);

    float brightnessFactor = 1.f - std::min(rowDistance / (tile_size * max_linear_attenuation_distance), 1.f);
    uint8_t bright = static_cast<uint8_t>(std::clamp(brightnessFactor * 255.f, 30.f, 255.f));
    sf::Color rowColor(bright, bright, bright);

    va[vaIndex + 0].position = sf::Vector2f(0.f, static_cast<float>(y));
    va[vaIndex + 0].texCoords = sf::Vector2f(uLeft, vLeft);
    va[vaIndex + 0].color = rowColor;

    va[vaIndex + 1].position = sf::Vector2f(static_cast<float>(SCREEN_WIDTH), static_cast<float>(y));
    va[vaIndex + 1].texCoords = sf::Vector2f(uRight, vRight);
    va[vaIndex + 1].color = rowColor;

    float yBottom = static_cast<float>(std::min(y + ROW_STEP, ROW_END));
    va[vaIndex + 2].position = sf::Vector2f(static_cast<float>(SCREEN_WIDTH), yBottom);
    va[vaIndex + 2].texCoords = sf::Vector2f(uRight, vRight);
    va[vaIndex + 2].color = rowColor;

    va[vaIndex + 3].position = sf::Vector2f(0.f, yBottom);
    va[vaIndex + 3].texCoords = sf::Vector2f(uLeft, vLeft);
    va[vaIndex + 3].color = rowColor;

    vaIndex += 4;
  }

  window.draw(va, states);
}


void RenderSystem::renderWalls(Registry &registry, Configuration config, const Entity& tilemapEntity, float playerAngle, const RayCastResultComponent& rayResults, float globalTime, const TextureManager& textureManager, std::vector<RenderItem>& items)
{
  auto* tilemapComp = registry.getComponent<TilemapComponent>(tilemapEntity);
  if (!tilemapComp || rayResults.hits.empty()) return;

  const auto amount_of_rays = static_cast<unsigned>(config.resolution_option);
  const auto fov = config.fov;
  const auto halfFov = fov / 2.f;
  const auto deltaAngle = fov / static_cast<float>(amount_of_rays);
  const auto maxAttenuationDist = config.attenuation_distance * config.tile_size;
  const bool fishEyeCorrection = config.enable_fish_eye;

  const float screenDist = HALF_SCREEN_WIDTH / std::tan(halfFov);
  const float columnWidth = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(amount_of_rays);
  const float ang = radiansFromDegrees(playerAngle);
  float rayAngle = ang - halfFov;

  std::unordered_map<std::string, const sf::Texture*> textureCache;

  for (unsigned i = 0; i < amount_of_rays && i < rayResults.hits.size(); ++i)
  {
    const RayHit& hit = rayResults.hits[i];
    if (hit.distance <= 0.f || !std::isfinite(hit.distance))
    {
      rayAngle += deltaAngle;
      continue;
    }

    const float rayA = hit.rayAngle;

    float correctedDepth = hit.distance;
    if (fishEyeCorrection)
    {
      correctedDepth *= std::cos(ang - rayA);
    }

    const float projHeight = screenDist * config.tile_size / (correctedDepth + BIG_EPSILON);
    const float h = std::min(projHeight, static_cast<float>(SCREEN_HEIGHT) * 2.f);
    const float columnX = static_cast<float>(i) * columnWidth;
    const float columnY = HALF_SCREEN_HEIGHT - (h / 2.f);

    if (hit.tileX < 0 || hit.tileY < 0 ||
        hit.tileX >= static_cast<int>(tilemapComp->width) ||
        hit.tileY >= static_cast<int>(tilemapComp->height))
    {
      if (const sf::Texture* solidTex = textureManager.get("placeholder"))
      {
        sf::Sprite sprite(*solidTex);
        sprite.setScale(columnWidth, h);
        sprite.setPosition(columnX, columnY);
        float brightness = 1.f - std::min(correctedDepth / maxAttenuationDist, 1.f);
        uint8_t bright = static_cast<uint8_t>(std::clamp(brightness * 255.f, 30.f, 255.f));
        sprite.setColor(sf::Color(bright, bright, bright));
        RenderItem it; it.depth = correctedDepth; it.sprite = sprite;
        items.push_back(std::move(it));
      }
      rayAngle += deltaAngle;
      continue;
    }

    const char tileChar = tilemapComp->tiles[hit.tileY][hit.tileX];
    auto appIt = tilemapComp->tileAppearanceMap.find(tileChar);

    if (appIt == tilemapComp->tileAppearanceMap.end())
    {
      if (const sf::Texture* solidTex = textureManager.get("placeholder"))
      {
        sf::Sprite sprite(*solidTex);
        sprite.setScale(columnWidth, h);
        sprite.setPosition(columnX, columnY);
        float brightness = 1.f - std::min(correctedDepth / maxAttenuationDist, 1.f);
        uint8_t bright = static_cast<uint8_t>(std::clamp(brightness * 255.f, 30.f, 255.f));
        sprite.setColor(sf::Color(bright, bright, bright));
        RenderItem it; it.depth = correctedDepth; it.sprite = sprite;
        items.push_back(std::move(it));
      }
      rayAngle += deltaAngle;
      continue;
    }

    const TileAppearance& appearance = appIt->second;
    const std::string& texId = appearance.isAnimated() ?
                              appearance.currentTextureId(globalTime) :
                              appearance.singleTextureId;

    if (texId.empty())
    {
      if (const sf::Texture* solidTex = textureManager.get("placeholder"))
      {
        sf::Sprite sprite(*solidTex);
        sprite.setScale(columnWidth, h);
        sprite.setPosition(columnX, columnY);
        float brightness = 1.f - std::min(correctedDepth / maxAttenuationDist, 1.f);
        uint8_t bright = static_cast<uint8_t>(std::clamp(brightness * 255.f, 30.f, 255.f));
        sprite.setColor(sf::Color(bright, bright, bright));
        RenderItem it; it.depth = correctedDepth; it.sprite = sprite;
        items.push_back(std::move(it));
      }
      rayAngle += deltaAngle;
      continue;
    }

    const sf::Texture* tex = nullptr;
    if (auto cacheIt = textureCache.find(texId); cacheIt != textureCache.end()) {
      tex = cacheIt->second;
    } else {
      tex = textureManager.get(texId);
      textureCache[texId] = tex;
    }

    if (!tex)
    {
      if (const sf::Texture* solidTex = textureManager.get("placeholder"))
      {
        sf::Sprite sprite(*solidTex);
        sprite.setScale(columnWidth, h);
        sprite.setPosition(columnX, columnY);
        float brightness = 1.f - std::min(correctedDepth / maxAttenuationDist, 1.f);
        uint8_t bright = static_cast<uint8_t>(std::clamp(brightness * 255.f, 30.f, 255.f));
        sprite.setColor(sf::Color(bright, bright, bright));
        RenderItem it; it.depth = correctedDepth; it.sprite = sprite;
        items.push_back(std::move(it));
      }
      rayAngle += deltaAngle;
      continue;
    }

    float u;
    {
      const float tileSizeF = config.tile_size;
      if (hit.vertical)
      {
        const float hitYTiles = hit.hitPointWorld.y / tileSizeF;
        float frac = hitYTiles - std::floor(hitYTiles);
        if (frac < 0.f) frac += 1.f;
        u = std::cos(rayA) > 0.f ? frac : (1.f - frac);
      }
      else
      {
        const float hitXTiles = hit.hitPointWorld.x / tileSizeF;
        float frac = hitXTiles - std::floor(hitXTiles);
        if (frac < 0.f) frac += 1.f;
        u = std::sin(rayA) > 0.f ? (1.f - frac) : frac;
      }
    }

    const int texWidth = static_cast<int>(tex->getSize().x);
    const int texHeight = static_cast<int>(tex->getSize().y);

    int sampleCenter = static_cast<int>(std::floor(u * static_cast<float>(texWidth) + 0.0001f)) % texWidth;
    if (sampleCenter < 0) sampleCenter += texWidth;

    const float projTileWidth = screenDist * config.tile_size / (correctedDepth + BIG_EPSILON);
    const float texelsPerPixel = texWidth / (projTileWidth + BIG_EPSILON);
    const int sourceWidth = std::max(1, static_cast<int>(std::round(texelsPerPixel * columnWidth)));

    int sx = sampleCenter - sourceWidth / 2;
    sx = std::clamp(sx, 0, texWidth - sourceWidth);

    sf::IntRect rect(sx, 0, sourceWidth, texHeight);
    sf::Sprite sprite(*tex, rect);

    const float scaleX = columnWidth / static_cast<float>(rect.width);
    const float scaleY = h / static_cast<float>(rect.height);
    sprite.setScale(scaleX, scaleY);
    sprite.setPosition(columnX, columnY);

    const float brightness = 1.f - std::min(correctedDepth / maxAttenuationDist, 1.f);
    const uint8_t bright = static_cast<uint8_t>(std::clamp(brightness * 255.f, 30.f, 255.f));
    sprite.setColor(sf::Color(bright, bright, bright));

    RenderItem it;
    it.depth = correctedDepth;
    it.sprite = sprite;
    items.push_back(std::move(it));

    rayAngle += deltaAngle;
  }
}

void RenderSystem::drawSolidColumn(sf::RenderWindow &window, const float x, const  float y, const float width, const float height, const float depth, const float maxAttenuationDist)
{
  const float brightness = 1.f - std::min(depth / maxAttenuationDist, 1.f);
  const uint8_t bright = static_cast<uint8_t>(std::clamp(brightness * 255.f, 30.f, 255.f));
  sf::RectangleShape column(sf::Vector2f(std::ceil(width), height));
  column.setPosition(x, y);
  column.setFillColor(sf::Color(bright, bright, bright));
  window.draw(column);
}


void RenderSystem::renderEnemies(Registry &registry, const Configuration &config, const TextureManager &textureManager, std::vector<RenderItem>& items)
{
  Entity player = INVALID_ENTITY;
  for (const auto &e : registry.entities())
  {
    if (registry.hasComponent<PlayerTag>(e))
    {
      player = e;
      break;
    }
  }
  if (player == INVALID_ENTITY) return;

  const auto* posComp = registry.getComponent<PositionComponent>(player);
  const auto* rotComp = registry.getComponent<RotationComponent>(player);
  const auto* rayResults = registry.getComponent<RayCastResultComponent>(player);
  if (!posComp || !rotComp || !rayResults) return;
  if (rayResults->hits.empty()) return;

  const int       amount_of_rays = static_cast<int>(rayResults->hits.size());
  const float     fov            = config.fov;
  const float     halfFov        = fov * 0.5f;
  const float     deltaAngle     = fov / static_cast<float>(amount_of_rays);
  constexpr auto  windowW        = static_cast<float>(SCREEN_WIDTH);
  constexpr auto  windowH        = static_cast<float>(SCREEN_HEIGHT);
  const float     screenDist     = (windowW * 0.5f) / std::tan(halfFov);
  const float     columnWidth    = windowW / static_cast<float>(amount_of_rays);

  struct EnemyEntry { Entity e; float dist; const PositionComponent* pos; const EnemyComponent* comp; };
  std::vector<EnemyEntry> enemies;
  for (const auto &ent : registry.entities())
  {
    if (!registry.hasComponent<EnemyTag>(ent)) continue;
    const auto* epos = registry.getComponent<PositionComponent>(ent);
    const auto* enemyComp = registry.getComponent<EnemyComponent>(ent);
    if (!epos || !enemyComp) continue;
    const float dx = epos->position.x - posComp->position.x;
    const float dy = epos->position.y - posComp->position.y;
    const float enemyDist = std::hypot(dx, dy);
    if (!std::isfinite(enemyDist) || enemyDist <= 0.f) continue;
    enemies.push_back(EnemyEntry{ent, enemyDist, epos, enemyComp});
  }

  std::ranges::sort(enemies, [](auto &a, auto &b){ return a.dist > b.dist; });

  const double playerAngleRad = radiansFromDegrees(rotComp->angle);

  std::unordered_map<std::string, const sf::Texture*> textureCache;
  for (const auto &entry : enemies)
  {
    const auto* epos = entry.pos;
    const auto* enemyComp = entry.comp;
    const float dx = epos->position.x - posComp->position.x;
    const float dy = epos->position.y - posComp->position.y;
    const float enemyDist = entry.dist;

    double angleToEnemy = std::atan2(static_cast<double>(dy), static_cast<double>(dx));
    double delta = angleToEnemy - playerAngleRad;
    while (delta > M_PI) delta -= 2.0 * M_PI;
    while (delta < -M_PI) delta += 2.0 * M_PI;

    if (std::abs(delta) > static_cast<double>(halfFov)) continue;

    int centerRay = static_cast<int>(std::floor((delta + static_cast<double>(halfFov)) / static_cast<double>(deltaAngle)));
    centerRay = std::clamp(centerRay, 0, amount_of_rays - 1);

    const auto &hit = rayResults->hits[centerRay];
    if (hit.distance <= 0.f || !std::isfinite(hit.distance)) continue;

    if (enemyDist + SMALL_EPSILON >= hit.distance) continue;

    const auto normDist = static_cast<float>(enemyDist * std::cos(delta));
    if (normDist <= SMALL_EPSILON) continue;

    const float projHeight = screenDist * config.tile_size / (normDist + SMALL_EPSILON);
    const float projWidth = projHeight * 0.75f * enemyComp->spriteScale;
    const float screenX = ((static_cast<float>((delta + halfFov) / deltaAngle)) * columnWidth);
    const float spriteX = screenX - (projWidth * 0.5f);
    const float spriteY = (windowH * 0.5f) - (projHeight * 0.5f) + (projHeight * enemyComp->heightShift);

    if (const auto* sc = registry.getComponent<SpriteComponent>(entry.e); sc && !sc->textureFrames.empty())
    {
      const std::size_t idx = std::min(sc->currentFrame, sc->textureFrames.size() - 1);
      const std::string& frameTexId = sc->textureFrames[idx];

      const sf::Texture* tex = nullptr;
      if (auto it = textureCache.find(frameTexId); it != textureCache.end()) tex = it->second;
      else { tex = textureManager.get(frameTexId); textureCache[frameTexId] = tex; }

      if (tex)
      {
        sf::Sprite sprite(*tex);

        const float texW = static_cast<float>(tex->getSize().x);
        const float texH = static_cast<float>(tex->getSize().y);

        const float scaleX = (projWidth / std::max(1.f, texW)) * enemyComp->spriteScale;
        const float scaleY = (projHeight / std::max(1.f, texH)) * enemyComp->spriteScale;
        sprite.setScale(scaleX, scaleY);
        sprite.setPosition(spriteX, spriteY);

        const float maxAttenuation = config.attenuation_distance * config.tile_size;
        const float brightness = 1.f - std::min(normDist / maxAttenuation, 1.f);
        const uint8_t bright = static_cast<uint8_t>(std::clamp(brightness * 255.f, 30.f, 255.f));
        sprite.setColor(sf::Color(bright, bright, bright));

        RenderItem it; it.depth = normDist; it.sprite = sprite; items.push_back(std::move(it));
      }
      else
      {
        if (const sf::Texture* solidTex = textureManager.get("placeholder"))
        {
          sf::Sprite sprite(*solidTex);
          sprite.setScale(projWidth, projHeight);
          sprite.setPosition(spriteX, spriteY);
          const float maxAttenuation = config.attenuation_distance * config.tile_size;
          const float brightness = 1.f - std::min(normDist / maxAttenuation, 1.f);
          const uint8_t bright = static_cast<uint8_t>(std::clamp(brightness * 255.f, 30.f, 255.f));
          sprite.setColor(sf::Color(bright, bright, bright));
          RenderItem it; it.depth = normDist; it.sprite = sprite; items.push_back(std::move(it));
        }
      }
    }
    else if (sc && !sc->frames.empty() && !sc->textureId.empty())
    {
      const std::size_t idx = std::min(sc->currentFrame, sc->frames.size() - 1);
      const sf::IntRect rect = sc->frames[idx];
      const std::string& texId = sc->textureId;

      const sf::Texture* tex = nullptr;
      if (auto it = textureCache.find(texId); it != textureCache.end()) tex = it->second;
      else { tex = textureManager.get(texId); textureCache[texId] = tex; }

      if (tex)
      {
        sf::Sprite sprite(*tex, rect);

        const float rectW = static_cast<float>(std::max(1, rect.width));
        const float rectH = static_cast<float>(std::max(1, rect.height));
        const float scaleX = (projWidth / rectW) * enemyComp->spriteScale;
        const float scaleY = (projHeight / rectH) * enemyComp->spriteScale;
        sprite.setScale(scaleX, scaleY);

        sprite.setPosition(spriteX, spriteY);

        const float maxAttenuation = config.attenuation_distance * config.tile_size;
        const float brightness = 1.f - std::min(normDist / maxAttenuation, 1.f);
        const uint8_t bright = static_cast<uint8_t>(std::clamp(brightness * 255.f, 30.f, 255.f));
        sprite.setColor(sf::Color(bright, bright, bright));

        RenderItem it; it.depth = normDist; it.sprite = sprite; items.push_back(std::move(it));
      }
    }
    else
    {
      if (!enemyComp->textureId.empty())
      {
        const std::string& texId = enemyComp->textureId;
        const sf::Texture* tex = nullptr;
        if (auto it = textureCache.find(texId); it != textureCache.end()) tex = it->second;
        else { tex = textureManager.get(texId); textureCache[texId] = tex; }

        if (tex)
        {
          sf::Sprite sprite;
          sprite.setTexture(*tex);

          const float texW = static_cast<float>(tex->getSize().x);
          const float texH = static_cast<float>(tex->getSize().y);

          const float scaleX = (projWidth / texW) * enemyComp->spriteScale;
          const float scaleY = (projHeight / texH) * enemyComp->spriteScale;
          sprite.setScale(scaleX, scaleY);

          sprite.setPosition(spriteX, spriteY);

          const float maxAttenuation = config.attenuation_distance * config.tile_size;
          const float brightness = 1.f - std::min(normDist / maxAttenuation, 1.f);
          const uint8_t bright = static_cast<uint8_t>(std::clamp(brightness * 255.f, 30.f, 255.f));
          sprite.setColor(sf::Color(bright, bright, bright));

          RenderItem it; it.depth = normDist; it.sprite = sprite; items.push_back(std::move(it));
        }
        else
        {
          if (const sf::Texture* solidTex = textureManager.get("placeholder"))
          {
            sf::Sprite sprite(*solidTex);
            sprite.setScale(projWidth, projHeight);
            sprite.setPosition(spriteX, spriteY);
            const float maxAttenuation = config.attenuation_distance * config.tile_size;
            const float brightness = 1.f - std::min(normDist / maxAttenuation, 1.f);
            const uint8_t bright = static_cast<uint8_t>(std::clamp(brightness * 255.f, 30.f, 255.f));
            sprite.setColor(sf::Color(bright, bright, bright));
            RenderItem it; it.depth = normDist; it.sprite = sprite; items.push_back(std::move(it));
          }
        }
      }
      else
      {
        if (const sf::Texture* solidTex = textureManager.get("placeholder"))
        {
          sf::Sprite sprite(*solidTex);
          sprite.setScale(projWidth, projHeight);
          sprite.setPosition(spriteX, spriteY);
          const float maxAttenuation = config.attenuation_distance * config.tile_size;
          const float brightness = 1.f - std::min(normDist / maxAttenuation, 1.f);
          const uint8_t bright = static_cast<uint8_t>(std::clamp(brightness * 255.f, 30.f, 255.f));
          sprite.setColor(sf::Color(bright, bright, bright));
          RenderItem it; it.depth = normDist; it.sprite = sprite; items.push_back(std::move(it));
        }
      }
    }
  }
}



void RenderSystem::renderProjectiles(Registry &registry, const Configuration &config, const TextureManager &textureManager, std::vector<RenderItem>& items)
{
  Entity player = INVALID_ENTITY;
  for (const auto &e : registry.entities())
  {
    if (registry.hasComponent<PlayerTag>(e))
    {
      player = e;
      break;
    }
  }
  if (player == INVALID_ENTITY) return;

  const auto* posComp = registry.getComponent<PositionComponent>(player);
  const auto* rotComp = registry.getComponent<RotationComponent>(player);
  const auto* rayResults = registry.getComponent<RayCastResultComponent>(player);
  if (!posComp || !rotComp || !rayResults) return;
  if (rayResults->hits.empty()) return;

  const int amount_of_rays = static_cast<int>(rayResults->hits.size());
  const float fov = config.fov;
  const float halfFov = fov * 0.5f;
  const float deltaAngle = fov / static_cast<float>(amount_of_rays);
  constexpr auto windowW = static_cast<float>(SCREEN_WIDTH);
  constexpr auto windowH = static_cast<float>(SCREEN_HEIGHT);
  const float screenDist = (windowW * 0.5f) / std::tan(halfFov);
  const float columnWidth = windowW / static_cast<float>(amount_of_rays);

  struct ProjEntry { Entity e; float dist; const PositionComponent* pos; const ProjectileComponent* comp; };
  std::vector<ProjEntry> projs;
  projs.reserve(64);

  for (const auto &ent : registry.entities())
  {
    if (!registry.hasComponent<ProjectileTag>(ent)) continue;
    const auto* ppos = registry.getComponent<PositionComponent>(ent);
    const auto* pc = registry.getComponent<ProjectileComponent>(ent);
    if (!ppos || !pc) continue;

    const float dx = ppos->position.x - posComp->position.x;
    const float dy = ppos->position.y - posComp->position.y;
    const float dist = std::hypot(dx, dy);
    if (!std::isfinite(dist) || dist <= 0.f) continue;

    projs.push_back(ProjEntry{ent, dist, ppos, pc});
  }

  if (projs.empty()) return;

  std::ranges::sort(projs, [](const ProjEntry& a, const ProjEntry& b){ return a.dist > b.dist; });

  const double playerAngleRad = radiansFromDegrees(rotComp->angle);

  std::unordered_map<std::string, const sf::Texture*> textureCache;

  for (const auto &entry : projs)
  {
    const auto* ppos = entry.pos;
    const auto* pc = entry.comp;

    if (pc->textureId.empty()) continue;

    const float dx = ppos->position.x - posComp->position.x;
    const float dy = ppos->position.y - posComp->position.y;
    const float projDist = entry.dist;

    double angleTo = std::atan2(static_cast<double>(dy), static_cast<double>(dx));
    double delta = angleTo - playerAngleRad;
    while (delta > M_PI) delta -= 2.0 * M_PI;
    while (delta < -M_PI) delta += 2.0 * M_PI;

    if (std::abs(delta) > static_cast<double>(halfFov)) continue;

    int centerRay = static_cast<int>(std::floor((delta + static_cast<double>(halfFov)) / static_cast<double>(deltaAngle)));
    centerRay = std::clamp(centerRay, 0, amount_of_rays - 1);

    const auto &hit = rayResults->hits[centerRay];
    if (hit.distance <= 0.f || !std::isfinite(hit.distance)) continue;

    if (projDist + SMALL_EPSILON >= hit.distance) continue;

    const auto normDist = static_cast<float>(projDist * std::cos(delta));
    if (normDist <= SMALL_EPSILON) continue;

    const float projTileSize = config.tile_size * std::max(0.05f, pc->visualSizeTiles);
    const float projHeight = screenDist * projTileSize / (normDist + SMALL_EPSILON);
    const float projWidth  = projHeight;

    const float screenX = ((static_cast<float>((delta + halfFov) / deltaAngle)) * columnWidth);
    const float spriteX = screenX - (projWidth * 0.5f);
    const float spriteY = (windowH * 0.5f) - (projHeight * 0.5f) + (projHeight * pc->heightShift);

    const sf::Texture* tex = nullptr;
    if (auto it = textureCache.find(pc->textureId); it != textureCache.end()) tex = it->second;
    else { tex = textureManager.get(pc->textureId); textureCache[pc->textureId] = tex; }

    if (!tex) continue;

    sf::Sprite sprite(*tex);

    const float texW = static_cast<float>(std::max(1u, tex->getSize().x));
    const float texH = static_cast<float>(std::max(1u, tex->getSize().y));

    const float scaleX = (projWidth / texW) * pc->spriteScale;
    const float scaleY = (projHeight / texH) * pc->spriteScale;

    sprite.setScale(scaleX, scaleY);
    sprite.setPosition(spriteX, spriteY);

    const float maxAttenuation = config.attenuation_distance * config.tile_size;
    const float brightness = 1.f - std::min(normDist / maxAttenuation, 1.f);
    const uint8_t bright = static_cast<uint8_t>(std::clamp(brightness * 255.f, 40.f, 255.f));
    sprite.setColor(sf::Color(bright, bright, bright));

    RenderItem it;
    it.depth = normDist;
    it.sprite = sprite;
    items.push_back(std::move(it));
  }
}
