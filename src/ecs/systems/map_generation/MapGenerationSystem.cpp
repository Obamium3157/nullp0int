//
// Created by obamium3157 on 30.11.2025.
//

#include "MapGenerationSystem.h"
#include <cctype>
#include <filesystem>
#include <fstream>
#include <queue>
#include <random>
#include <vector>

bool Point::operator==(const Point &p) const
{
  return x == p.x && y == p.y;
}

bool Point::operator<(const Point &p) const
{
  return cost > p.cost;
}

bool Room::intersect(const Room &other) const
{
  return !(other.x >= (x + w) || x >= (other.x + other.w) || other.y >= (y + h) || y >= (other.y + other.h));
}

MapGenerationSystem::MapGenerationSystem(const int width, const int height, const uint32_t seed)
    : m_width(width), m_height(height), m_rng(seed)
{
  m_data.resize(width * height, '#');
}

void MapGenerationSystem::generatePassage(const Point &start, const Point &finish)
{

  auto isProtected = [this](const int px, const int py) -> bool {
    if (px < 0 || py < 0 || px >= m_width || py >= m_height) return false;
    const char ch = m_data[px + py * m_width];
    return static_cast<bool>(std::isupper(static_cast<unsigned char>(ch)));
  };

  std::vector parents(m_width * m_height, -1);

  std::priority_queue<Point> active;
  active.push(start);

  static constexpr int directions[4][2] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
  while (!active.empty())
  {
    const Point point = active.top();
    active.pop();

    if (point == finish) break;

    for (int i = 0; i < 4; ++i)
    {
      Point p = { point.x - directions[i][0], point.y - directions[i][1], 0 };
      if (p.x < 0 || p.y < 0 || p.x >= m_width || p.y >= m_height) continue;

      if (isProtected(p.x, p.y)) continue;

      if (parents[p.x + p.y * m_width] < 0)
      {
        p.cost = calcCost(p, finish);
        active.push(p);

        parents[p.x + p.y * m_width] = i;
      }
    }
  }

  if (parents[finish.x + finish.y * m_width] < 0)
  {
    return;
  }

  Point point = finish;
  while (point != start)
  {
    char &cell = m_data[point.x + point.y * m_width];
    if (cell != '*' && cell != '>' && !std::isupper(static_cast<unsigned char>(cell)))
    {
      cell = ' ';
    }

    const int parentIndex = parents[point.x + point.y * m_width];
    if (parentIndex < 0) break;

    const int *direction = directions[parentIndex];
    point.x += direction[0];
    point.y += direction[1];
  }
}

void MapGenerationSystem::connectRooms()
{
  if (m_rooms.size() < 2) return;

  for (size_t i = 1; i < m_rooms.size(); ++i) {
    Point start{
      m_rooms[i-1].x + m_rooms[i-1].w / 2,
      m_rooms[i-1].y + m_rooms[i-1].h / 2,
      0
    };

    Point finish{
      m_rooms[i].x + m_rooms[i].w / 2,
      m_rooms[i].y + m_rooms[i].h / 2,
      0
    };

    generatePassage(start, finish);
  }
}

void MapGenerationSystem::generate(const int roomsCount)
{
  m_rooms.clear();

  std::uniform_int_distribution roomTypeDist;
  std::uniform_int_distribution presetIndexDist(0, static_cast<int>(normalPresets.size()) - 1);

  for (int i = 0; i < roomsCount; ++i)
  {
    RoomPreset preset;
    int w, h;

    if (i == 0)
    {
      preset = entrancePreset;
      w = static_cast<int>(entrancePreset[0].size());
      h = static_cast<int>(entrancePreset.size());
    }
    else if (i == roomsCount - 1)
    {
      preset = exitPreset;
      w = static_cast<int>(exitPreset[0].size());
      h = static_cast<int>(exitPreset.size());
    }
    else
    {
      const int idx = presetIndexDist(m_rng);
      preset = normalPresets[idx];
      h = static_cast<int>(preset.size());
      w = h > 0 ? static_cast<int>(preset[0].size()) : 1;

      for (const auto &row : preset)
      {
        if (static_cast<int>(row.size()) > w) w = static_cast<int>(row.size());
      }
    }

    constexpr int minX = 3;
    constexpr int minY = 3;
    int maxX = m_width - w - 4;
    int maxY = m_height - h - 4;
    if (maxX < minX) maxX = minX;
    if (maxY < minY) maxY = minY;

    auto xPosDist = std::uniform_int_distribution(minX, maxX);
    auto yPosDist = std::uniform_int_distribution(minY, maxY);

    for (int j = 0; j < 1000; ++j)
    {
      const Room room = {
        xPosDist(m_rng),
        yPosDist(m_rng),
        w,
        h,
        preset
      };

      bool intersects = false;
      for (const auto& existing : m_rooms)
      {
        if (room.intersect(existing))
        {
          intersects = true;
          break;
        }
      }

      if (!intersects)
      {
        m_rooms.push_back(room);
        break;
      }
    }
  }
}


void MapGenerationSystem::build()
{
  m_data.assign(m_width * m_height, '#');
  for (const Room &room : m_rooms)
  {
    for (int local_y = 0; local_y < room.h; ++local_y)
    {
      for (int local_x = 0; local_x < room.w; ++local_x)
      {
        if (local_y < static_cast<int>(room.preset.size()) &&
            local_x < static_cast<int>(room.preset[local_y].size()))
        {
          const char c = room.preset[local_y][local_x];
          const int global_x = room.x + local_x;
          const int global_y = room.y + local_y;

          if (global_x >= 0 && global_x < m_width &&
              global_y >= 0 && global_y < m_height)
          {
            m_data[global_x + global_y * m_width] = c;
          }
        }
      }
    }
  }
}

std::string MapGenerationSystem::print()
{
  const std::string filename = "map_" + generateRandomString() + ".txt";
  std::ofstream file("resources/maps/generated/" + filename);

  for (int y = 0; y < m_height; ++y)
  {
    for (int x = 0; x < m_width; ++x)
    {
      file << m_data[x + y * m_width];
    }
    file << "\n";
  }

  return filename;
}

int MapGenerationSystem::calcCost(const Point &a, const Point &b)
{
  return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

std::string MapGenerationSystem::generateLevel(const int amountOfRooms)
{
  this->generate(amountOfRooms);
  this->build();
  this->connectRooms();
  return this->print();
}

std::string MapGenerationSystem::generateRandomString()
{
  constexpr size_t RANDOM_STRING_LENGTH = 10;

  const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  std::uniform_int_distribution<size_t> distribution(0, charset.length() - 1);

  std::string randomString;
  randomString.reserve(RANDOM_STRING_LENGTH);

  for (size_t i = 0; i < RANDOM_STRING_LENGTH; ++i) {
    randomString += charset[distribution(m_rng)];
  }

  return randomString;
}
