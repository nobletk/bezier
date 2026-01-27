#ifndef UTILS_H
#define UTILS_H

#include "BezierMath.h"
#include "Point.h"
#include <GL/freeglut.h>
#include <iostream>
#include <vector>

struct BezierCurve {
  std::vector<Point> controlPoints;
  int resolution;
  bool isSelected;

  BezierCurve() {
    resolution = 100;
    isSelected = false;
  }
};

struct BezierSurface {
  std::vector<Point> controlPoints;
  int resolution; // Mesh grid resolution
  bool isSelected;

  BezierSurface() {
    resolution = 20;
    isSelected = false;
  }
};

// Function that calculates the squared distance between points.
inline GLfloat distanceSquared(Point A, Point B) {
  return pow(A.getX() - B.getX(), 2) + pow(A.getY() - B.getY(), 2);
}

// Function that returns the index of the active point in 2D mode.
inline GLint getActivePoint2D(const std::vector<Point> p, GLint sens, GLint x,
                              GLint y) {
  GLint s = sens * sens;
  Point P = Point((float)x, (float)y, 0.0);

  for (size_t i = 0; i < p.size(); i++) {
    if (distanceSquared(p[i], P) < s) {
      return i;
    }
  }
  return -1;
}

// Function that calculates the squared distance between point P and a Line
// Segment AB
inline float distPointToLine(Point P, Point A, Point B) {
  // Vector AB
  float abX = B.getX() - A.getX();
  float abY = B.getY() - A.getY();
  float abZ = B.getZ() - A.getZ();

  // Vector AP
  float apX = P.getX() - A.getX();
  float apY = P.getY() - A.getY();
  float apZ = P.getZ() - A.getZ();

  // Project AP onto AB
  float t =
      (apX * abX + apY * abY + apZ * abZ) / (abX * abX + abY * abY + abZ * abZ);

  // Clamp t to segment [0, 1] to keep it between Near and Far
  if (t < 0.0f) {
    t = 0.0f;
  }
  if (t > 1.0f) {
    t = 1.0f;
  }

  // Closest point on the line
  float closeX = A.getX() + abX * t;
  float closeY = A.getY() + abY * t;
  float closeZ = A.getZ() + abZ * t;

  // Squared Euclidean distance between P and that closest point
  return pow(P.getX() - closeX, 2) + pow(P.getY() - closeY, 2) +
         pow(P.getZ() - closeZ, 2);
}

// Function that snaps the mouse to existing points for 2D drawing.
inline Point getSnappedPoint2D(const std::vector<BezierCurve> &curves,
                               int xMouse, int yMouse, float r, float g,
                               float b, float pointSize) {
  for (const auto &curve : curves) {
    int hitIndex = getActivePoint2D(curve.controlPoints, 7.0, xMouse, yMouse);

    if (hitIndex != -1) {
      return curve.controlPoints[hitIndex];
    }
  }

  return Point((float)xMouse, (float)yMouse, 0.0f, r, g, b, pointSize);
}

// Function that tests curve selection based on curve approximation
inline bool isMouseOnCurve(const BezierCurve &curve, int xMouse, int yMouse) {
  Point mouseP((float)xMouse, (float)yMouse, 0.0f);

  float thresholdSquared = 25.0f;

  Point p1 = evaluateDeCasteljau(curve.controlPoints, 0.0f);

  for (int i = 1; i <= curve.resolution; ++i) {
    float t = (float)i / (float)curve.resolution;
    Point p2 = evaluateDeCasteljau(curve.controlPoints, t);

    if (distPointToLine(mouseP, p1, p2) < thresholdSquared) {
      return true;
    }

    p1 = p2;
  }

  return false;
}

// Function that finds all points that share the same position as the reference
// point
inline void
collectOverlappingPoints(const Point &referencePoint,
                         const std::vector<BezierCurve> &curves,
                         std::vector<std::pair<int, int>> &draggedPoints) {
  draggedPoints.clear();

  for (size_t c = 0; c < curves.size(); ++c) {
    for (size_t p = 0; p < curves[c].controlPoints.size(); ++p) {
      if (distanceSquared(curves[c].controlPoints[p], referencePoint) < 0.1f) {
        draggedPoints.push_back({(int)c, (int)p});
      }
    }
  }
}

// Function that converts raw screen pixel coordinates (Mouse) into Logical
// World coordinates.
inline Point getMouseWorldPosition(int xMouse, int yMouse, int winWidth,
                                   int winHeight, float logicalWidth,
                                   float logicalHeight) {

  if (winWidth == 0 || winHeight == 0) {
    return Point(0, 0, 0);
  }

  float worldX = (float)xMouse / (float)winWidth * logicalWidth;
  float worldY = (float)(winHeight - yMouse) / (float)winHeight * logicalHeight;
  return Point(worldX, worldY, 0.0f);
}

// Function that renders text on 2D scene
inline void renderText(float x, float y, void *font, const char *string) {
  const char *c;
  glRasterPos2f(x, y);
  for (c = string; *c != '\0'; c++) {
    glutBitmapCharacter(font, *c);
  }
}

// Function that prints helper to terminal for key interactions
inline void printInteraction(void) {
  std::cout << "Interaction:" << std::endl;
  std::cout << "Mouse Right Click for Menu Options." << std::endl;
  std::cout
      << "Mouse Left Click for moving/drawing points and selecting curves."
      << std::endl;
  std::cout << "Press Space to switch between 2D/3D." << std::endl;
  std::cout << "Press c to toggle showing control points." << std::endl;
  std::cout << "Press p to toggle showing control polygon." << std::endl;
  std::cout << "2D mode:" << std::endl;
  std::cout << "Press +/- to increase/decrease the number of control points "
               "for the selected curve."
            << std::endl;
  std::cout << "Press PageUp/PageDown to increase/decrease the resolution for "
               "the selected curve."
            << std::endl;
  std::cout << "Press d to switch between Edit/Draw mode." << std::endl;
  std::cout << "3D mode:" << std::endl;
  std::cout << "Press Up/Down/Left/Right Arrows to pan the camera."
            << std::endl;
  std::cout << "Press i/o to zoom the camera in/out." << std::endl;
  std::cout << "Press x/X to Rotate the camera on the X-Axis." << std::endl;
  std::cout << "Press y/Y to Rotate the camera on the Y-Axis." << std::endl;
  std::cout << "Press z/Z to Rotate the camera on the Z-Axis." << std::endl;
}

#endif
