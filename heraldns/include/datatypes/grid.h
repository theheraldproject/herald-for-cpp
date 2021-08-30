//  Copyright 2020-2021 Herald Project Contributors
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef GRID_H
#define GRID_H

#include <cstdint>
#include <memory>
#include <vector>

namespace heraldns {
namespace datatype {

class Presence; // fwd decl
class PresenceManager; // fwd decl

class Cell {
public:
  Cell(uint64_t x, uint64_t y); // randomise properties
  ~Cell() = default;

  void movedIn(uint64_t arrival);
  void movedOut(uint64_t leaver);

  uint64_t x() const;
  uint64_t y() const;

  const std::vector<uint64_t>& present() const;

private:
  uint64_t xPos;
  uint64_t yPos;
  std::vector<uint64_t> m_present;
};

class Grid {
public:
  Grid(std::uint64_t width,std::uint64_t height, double cellSeparationMetres);
  ~Grid() = default;

  void randomisePositions(const PresenceManager& pm) const;

  double separation() const;

  std::shared_ptr<Cell> cell(uint64_t x, uint64_t y) const;

  uint64_t height() const;

  uint64_t width() const;

  double distance(const std::shared_ptr<Cell>& c1, const std::shared_ptr<Cell>& c2) const;

private:
  uint64_t m_width;
  uint64_t m_height;
  std::vector<std::shared_ptr<Cell>> m_cells; // row (width) then column (height)
  double m_separation;
};

} // end namespace
} // end namespace

#endif