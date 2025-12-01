//
// Created by obamium3157 on 30.11.2025.
//

#ifndef NULLP0INT_MAPGENERATIONSYSTEM_H
#define NULLP0INT_MAPGENERATIONSYSTEM_H

#include <random>

#include "roomPresets.h"

#include <vector>

struct Point
{
  int x, y, cost;

  bool operator==(const Point &p) const;
  bool operator<(const Point &p) const;
};

struct Room
{
  int x, y, w, h;
  RoomPreset preset;

  [[nodiscard]] bool intersect(const Room &other) const;
};

class MapGenerationSystem
{
public:
  MapGenerationSystem(int width, int height, uint32_t seed);

  std::string generateLevel(int amountOfRooms);

private:
  int m_width, m_height;
  std::vector<char> m_data;
  std::vector<Room> m_rooms;
  std::mt19937 m_rng;

  void generatePassage(const Point &start, const Point &finish);
  void connectRooms();
  void generate(int roomsCount);
  void build();

  [[nodiscard]] std::string print();
  [[nodiscard]] static int calcCost(const Point &a, const Point &b);

  std::string generateRandomString();
};

#endif //NULLP0INT_MAPGENERATIONSYSTEM_H