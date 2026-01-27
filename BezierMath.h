#ifndef BEZIERMATH_H
#define BEZIERMATH_H

#include "Point.h"
#include <vector>

// Linear Interpolation
inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

// Iterative function to evalutate a Bezier curve using De Casteljau's
// Algorithm.
inline Point evaluateDeCasteljau(std::vector<Point> pts, float t) {
  if (pts.empty()) {
    return Point(0, 0, 0);
  }

  int n = pts.size();

  for (int k = 1; k < n; k++) {
    for (int i = 0; i < n - k; i++) {
      float newX = lerp(pts[i].getX(), pts[i + 1].getX(), t);
      float newY = lerp(pts[i].getY(), pts[i + 1].getY(), t);
      float newZ = lerp(pts[i].getZ(), pts[i + 1].getZ(), t);

      pts[i].setX(newX);
      pts[i].setY(newY);
      pts[i].setZ(newZ);
    }
  }

  return pts[0];
}

// Function to evaluate a point on a Bezier Surface as a "Curve of Curves".
inline Point evaluateBezierSurface(const std::vector<Point> &grid, float u,
                                   float v) {
  std::vector<Point> tempPoints;

  int gridSize = (int)sqrt(grid.size());
  if (gridSize * gridSize != grid.size()) {
    return Point(0, 0, 0);
  }

  for (int i = 0; i < gridSize; ++i) {
    std::vector<Point> row;

    for (int j = 0; j < gridSize; ++j) {
      row.push_back(grid[i * gridSize + j]);
    }

    tempPoints.push_back(evaluateDeCasteljau(row, u));
  }

  return evaluateDeCasteljau(tempPoints, v);
}

#endif
