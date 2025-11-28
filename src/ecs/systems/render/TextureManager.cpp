//
// Created by obamium3157 on 21.11.2025.
//

#include "TextureManager.h"

bool TextureManager::load(const std::string &id, const std::string &path)
{
  sf::Texture tex;
  if (!tex.loadFromFile(path))
  {
    return false;
  }
  tex.setSmooth(false);
  tex.setRepeated(true);
  m_textures[id] = tex;
  return true;
}

const sf::Texture *TextureManager::get(const std::string &id) const
{
  const auto it = m_textures.find(id);
  if (it == m_textures.end())
  {
    return nullptr;
  }
  return &it->second;
}
