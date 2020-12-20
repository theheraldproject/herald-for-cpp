//  Copyright 2020 VMware, Inc.
//  SPDX-License-Identifier: Apache-2.0
//

#include "../../heraldns.h"

#include <iostream>
#include <random>

using namespace heraldns;

namespace heraldns {
namespace datatype {


Cell::Cell(uint64_t x, uint64_t y)
  : xPos(x), yPos(y), m_present()
{
  ;
}

void
Cell::movedIn(uint64_t arrival)
{
  m_present.push_back(arrival);
}

void
Cell::movedOut(uint64_t leaver)
{
  auto result = std::find(m_present.begin(), m_present.end(), leaver);
  if (result != m_present.end()) {
    m_present.erase(result);
    
    //std::swap(*result, m_present.back());
    //m_present.pop_back();
  }
}

uint64_t
Cell::x() const
{
  return xPos;
}

uint64_t
Cell::y() const
{
  return yPos;
}

const std::vector<uint64_t>& 
Cell::present() const
{
  return m_present;
}





Grid::Grid(std::uint64_t width,std::uint64_t height, double cellSeparationMetres)
  : m_width(width), m_height(height), m_cells(), m_separation(cellSeparationMetres)
{
  // add new cell instances
  uint64_t cellCount = width * height;
  m_cells.reserve(cellCount);
  for (uint64_t y = 0;y < height;y++) {
    for (uint64_t x = 0;x < width;x++) {
      m_cells.push_back(std::make_shared<Cell>(x,y));
    }
  }
}

void
Grid::randomisePositions(const PresenceManager& pm) const
{
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<std::size_t> distrib(0, m_cells.size() - 1);

  for (uint64_t id = 0;id < pm.size();id++) {
    auto actor = pm.get(id);
    const std::shared_ptr<Cell>& pos = m_cells[distrib(gen)];
    actor->moveTo(pos);
  }
}

double
Grid::separation() const
{
  return m_separation;
}

std::shared_ptr<Cell> 
Grid::cell(uint64_t x, uint64_t y) const
{
  return m_cells[x + (y * m_width)];
}


uint64_t
Grid::height() const
{
  return m_height;
}

uint64_t
Grid::width() const
{
  return m_width;
}


double
Grid:: distance(const std::shared_ptr<Cell>& c1, const std::shared_ptr<Cell>& c2) const
{
  auto dx = (m_separation * c1->x()) - (m_separation * c2->x()); // convert from UNSIGNED to SIGNED
  auto dy = (m_separation * c1->y()) - (m_separation * c2->y()); // convert from UNSIGNED to SIGNED
  return sqrt((dx * dx) + (dy * dy));
}


}
}