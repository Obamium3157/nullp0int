//
// Created by obamium3157 on 21.11.2025.
//

#ifndef NULLP0INT_TEXTUREMANAGER_H
#define NULLP0INT_TEXTUREMANAGER_H

#pragma once
#include <string>
#include <unordered_map>
#include <SFML/Graphics.hpp>

class TextureManager
{
public:
  bool load(const std::string& id, const std::string& path);

  const sf::Texture* get(const std::string &id) const;

private:
  std::unordered_map<std::string, sf::Texture> m_textures;
};


#endif //NULLP0INT_TEXTUREMANAGER_H