// clang-format off
#include "BezierMath.h"
#include "Camera.h"
#include "Utils.h"
#include "Point.h"
#include "TeapotData.h"
#include "RocketData.h"
// clang-format on
#include <GL/freeglut_std.h>

#include <string>
#include <vector>

// Globals
// ============================================================================
// LOGICA DE EDICION DE VERTICES 2D EN EL PLANO
// Adaptado de: https://github.com/nobletk/bezier (Autor: nobletk)
// Tecnologia: C++ con OpenGL/GLUT
// Adaptacion: Se modifico la logica de interaccion y callbacks de seleccion y
// movimiento de puntos de control de Bezier para trabajar como edicion de
// vertices 2D libres en el plano, controlados por activacion/desactivacion (tecla V).
// ============================================================================
struct Vertex2D {
  float x;
  float y;
  Vertex2D(float x_ = 0.0f, float y_ = 0.0f) : x(x_), y(y_) {}
};

bool vertexEditEnabled = false;
std::vector<Vertex2D> vertices;
int selectedVertexIndex = -1;
const float selectionRadius = 15.0f;

enum AppMode { MODE_CURVE_2D, MODE_SURFACE_3D };

enum InteractionState {
  STATE_IDLE,
  STATE_DRAWING_READY,
  STATE_DRAWING_ACTIVE,
};

enum MenuOption {
  MENU_TOGGLE_GRID,
  MENU_TOGGLE_DRAW_MODE,
  MENU_TOGGLE_POINTS,
  MENU_TOGGLE_CONTROL_POLYGON,
  MENU_SWITCH_MODE,
  MENU_RESET_CAMERA,
  MENU_CLEAR,
  MENU_QUIT,
  MENU_DRAW_SURFACE,
  MENU_DRAW_TEAPOT,
  MENU_DRAW_ROCKET,
};

void menuHandler(int value);

int menuId2D, menuId3D, submenuIdObjects;

const float LOGICAL_WIDTH = 1000.0f;
const float LOGICAL_HEIGHT = 1000.0f;
static int winWidth, winHeight;
static float pointSize2D = 4.0;
static float pointSize3D = 2.0;
AppMode currentMode = MODE_CURVE_2D;
InteractionState currentState = STATE_IDLE;
GLint pointDragged = -1;
GLint activeCurveIndex = -1;
GLint activeSurfaceIndex = -1;
bool isDrawPoint = true;
bool isGrid = true;
bool isControlPolygon = true;

std::vector<std::pair<int, int>> draggedPoints;
std::vector<BezierCurve> allCurves;
std::vector<BezierSurface> allSurfaces;
Point tempLineStartPoint;
Camera camera;

// Cache of OpenGL matrices from the last frame in 3D mode.
// Required for gluUnProject() to calculate mouse rays correctly.
GLdouble model_view[16];
GLdouble projection[16];
GLint viewport[4];

// ============================================================================
// FUNCIONES DE CONTROL Y DIBUJO DE VERTICES (MODULO REUTILIZABLE)
// ============================================================================
void toggleVertexEditMode() {
  vertexEditEnabled = !vertexEditEnabled;
  if (vertexEditEnabled) {
    std::cout << "Modo edicion de vertices: ACTIVADO" << std::endl;
  } else {
    std::cout << "Modo edicion de vertices: DESACTIVADO" << std::endl;
    selectedVertexIndex = -1;
  }
  glutPostRedisplay();
}

void insertVertex(float x, float y) {
  vertices.push_back(Vertex2D(x, y));
  std::cout << "Vertice insertado en: (" << x << ", " << y << ")" << std::endl;
}

int findNearestVertex(float x, float y) {
  float minDistanceSq = selectionRadius * selectionRadius;
  int nearestIndex = -1;
  for (size_t i = 0; i < vertices.size(); ++i) {
    float distSq = pow(vertices[i].x - x, 2) + pow(vertices[i].y - y, 2);
    if (distSq < minDistanceSq) {
      minDistanceSq = distSq;
      nearestIndex = i;
    }
  }
  return nearestIndex;
}

void startMoveVertex(int index) {
  if (index >= 0 && index < (int)vertices.size()) {
    selectedVertexIndex = index;
    std::cout << "Vertice seleccionado para mover: indice " << index << std::endl;
  }
}

void moveSelectedVertex(float x, float y) {
  if (selectedVertexIndex >= 0 && selectedVertexIndex < (int)vertices.size()) {
    vertices[selectedVertexIndex].x = std::max(0.0f, std::min(x, LOGICAL_WIDTH));
    vertices[selectedVertexIndex].y = std::max(0.0f, std::min(y, LOGICAL_HEIGHT));
  }
}

void stopMoveVertex() {
  if (selectedVertexIndex != -1) {
    std::cout << "Vertice soltado en nueva posicion." << std::endl;
    selectedVertexIndex = -1;
  }
}

void drawVertices() {
  if (vertices.empty()) {
    return;
  }

  // 1. Dibujar la curva de Bezier generada por los vertices usando el algoritmo de De Casteljau
  if (vertices.size() > 1) {
    std::vector<Point> controlPts;
    for (const auto &v : vertices) {
      controlPts.push_back(Point(v.x, v.y, 0.0f));
    }

    glColor3f(0.85f, 0.15f, 0.15f); // Rojo elegante para la curva de Bezier generada
    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    int resolution = 100;
    for (int i = 0; i <= resolution; ++i) {
      float t = (float)i / (float)resolution;
      Point p = evaluateDeCasteljau(controlPts, t);
      glVertex3f(p.getX(), p.getY(), 0.0f);
    }
    glEnd();

    // 2. Dibujar el polígono de control (líneas que conectan los vértices)
    glColor3f(0.3f, 0.7f, 0.9f); // Azul cielo vibrante
    glLineWidth(1.0f);
    glBegin(GL_LINE_STRIP);
    for (const auto &v : vertices) {
      glVertex3f(v.x, v.y, 0.0f);
    }
    glEnd();
    glLineWidth(1.0f); // Resetear grosor
  }

  // 3. Dibujar los vertices individuales como puntos circulares
  for (size_t i = 0; i < vertices.size(); ++i) {
    float currentSize = 6.0f; // Tamaño normal
    
    // Si esta seleccionado, lo dibujamos mas grande y en color rojo
    if ((int)i == selectedVertexIndex) {
      glColor3f(1.0f, 0.3f, 0.3f); // Rojo brillante
      currentSize = 10.0f;
    } else {
      glColor3f(0.2f, 0.8f, 0.2f); // Verde brillante
    }

    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(vertices[i].x, vertices[i].y, 0.0f);
    for (int j = 0; j <= 30; ++j) {
      float t = 2.0f * 3.14159265f * (float)j / 30.0f;
      glVertex3f(vertices[i].x + currentSize * cos(t), vertices[i].y + currentSize * sin(t), 0.0f);
    }
    glEnd();
  }
}

// Function that draws all Bezier Curves
void drawBezierCurve(std::vector<BezierCurve> allCurves) {
  if (allCurves.empty()) {
    return;
  }

  for (auto &curve : allCurves) {
    // Change color of curve depending on mouse selection
    if (curve.isSelected) {
      glColor3f(1.0f, 0.0f, 0.0f);
      glLineWidth(3.0f);
    } else {
      glColor3f(0.0f, 0.0f, 0.0f);
      glLineWidth(2.0f);
    }

    // Draw the Bezier Curve
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= curve.resolution; ++i) {
      float t = (float)i / (float)curve.resolution;
      Point p = evaluateDeCasteljau(curve.controlPoints, t);
      glVertex3f(p.getX(), p.getY(), 0.0f);
    }
    glEnd();

    // Draw Control Polygon
    if (curve.controlPoints.size() > 1 && isControlPolygon) {
      glColor3f(0.7f, 0.7f, 0.7f);
      glLineWidth(1.0f);
      glBegin(GL_LINE_STRIP);
      for (const auto &p : curve.controlPoints) {
        glVertex3f(p.getX(), p.getY(), p.getZ());
      }
      glEnd();
    }

    // Draw the Control Points
    if (isDrawPoint) {
      for (auto &p : curve.controlPoints) {
        p.drawPoint2D();
      }
    }
  }
}

// Function that draws all Bezier Surfaces
void drawBezierSurface(const std::vector<BezierSurface> &surfaces) {
  if (surfaces.empty()) {
    return;
  }

  for (const auto &surface : surfaces) {
    // Change color of the surface depending on mouse selection
    if (surface.isSelected) {
      glColor3f(0.0f, 1.0f, 0.0f);
    } else {
      glColor3f(0.0f, 0.0f, 1.0f);
    }
    // Draw Mesh
    // Draw Horizontal lines of the Mesh
    for (int i = 0; i <= surface.resolution; ++i) {
      glBegin(GL_LINE_STRIP);
      for (int j = 0; j <= surface.resolution; ++j) {
        float u = (float)i / (float)surface.resolution;
        float v = (float)j / (float)surface.resolution;
        Point p = evaluateBezierSurface(surface.controlPoints, u, v);
        glVertex3f(p.getX(), p.getY(), p.getZ());
      }
      glEnd();
    }

    // Draw Vertical lines of the Mesh
    for (int j = 0; j <= surface.resolution; ++j) {
      glBegin(GL_LINE_STRIP);
      for (int i = 0; i <= surface.resolution; ++i) {
        float u = (float)i / (float)surface.resolution;
        float v = (float)j / (float)surface.resolution;
        Point p = evaluateBezierSurface(surface.controlPoints, u, v);
        glVertex3f(p.getX(), p.getY(), p.getZ());
      }
      glEnd();
    }

    // Draw Control Polygon
    if (isControlPolygon) {
      glColor3f(0.5f, 0.5f, 0.5f);
      glLineWidth(1.0f);

      int gridSize = (int)sqrt(surface.controlPoints.size());

      // Draw Horizontal Rows
      for (int i = 0; i < gridSize; ++i) {
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j < gridSize; ++j) {
          int index = i * gridSize + j;
          Point p = surface.controlPoints[index];
          glVertex3f(p.getX(), p.getY(), p.getZ());
        }
        glEnd();
      }

      // Draw  Vertical Columns
      for (int j = 0; j < gridSize; ++j) {
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i < gridSize; ++i) {
          int index = i * gridSize + j;
          Point p = surface.controlPoints[index];
          glVertex3f(p.getX(), p.getY(), p.getZ());
        }
        glEnd();
      }
    }

    // Draw Control Points
    if (isDrawPoint) {
      for (auto p : surface.controlPoints) {
        p.drawPoint3D();
      }
    }
  }
}

// Function that draws Grid for 2D scene
void drawGrid2D(void) {
  int i;

  glEnable(GL_LINE_STIPPLE);
  glLineStipple(1, 0x5555);
  glColor3f(0.75, 0.75, 0.75);

  glBegin(GL_LINES);
  for (i = 1; i < 10; i++) {
    float x = i * 0.1f * LOGICAL_WIDTH;
    glVertex3f(x, 0.0f, 0.0f);
    glVertex3f(x, LOGICAL_HEIGHT, 0.0f);
  }
  for (i = 0; i < 10; i++) {
    float y = i * 0.1f * LOGICAL_HEIGHT;
    glVertex3f(0.0f, y, 0.0f);
    glVertex3f(LOGICAL_WIDTH, y, 0.0f);
  }
  glEnd();
  glDisable(GL_LINE_STIPPLE);
}

// Function that draws 2D scene
void handleDrawScene2D(void) {
  glOrtho(0.0, LOGICAL_WIDTH, 0.0, LOGICAL_HEIGHT, -100.0, 100.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  // Capture OpenGL matrices from the last frame.
  glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetIntegerv(GL_VIEWPORT, viewport);

  // Draw Grid
  if (isGrid) {
    drawGrid2D();
  }

  drawBezierCurve(allCurves);

  // Dibujar los vertices e hilos creados en el modo de edicion 2D
  drawVertices();

  if (vertexEditEnabled) {
    // Modo Edición de Vértices ACTIVADO: Mostrar un panel limpio y bien delimitado
    glColor4f(0.95f, 0.95f, 0.95f, 0.8f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
      glVertex2f(5.0f, LOGICAL_HEIGHT - 5.0f);
      glVertex2f(420.0f, LOGICAL_HEIGHT - 5.0f);
      glVertex2f(420.0f, LOGICAL_HEIGHT - 105.0f);
      glVertex2f(5.0f, LOGICAL_HEIGHT - 105.0f);
    glEnd();
    glDisable(GL_BLEND);

    // Dibujar el borde del recuadro
    glColor3f(0.2f, 0.8f, 0.2f); // Borde verde
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
      glVertex2f(5.0f, LOGICAL_HEIGHT - 5.0f);
      glVertex2f(420.0f, LOGICAL_HEIGHT - 5.0f);
      glVertex2f(420.0f, LOGICAL_HEIGHT - 105.0f);
      glVertex2f(5.0f, LOGICAL_HEIGHT - 105.0f);
    glEnd();
    glLineWidth(1.0f);

    // Texto descriptivo para la interfaz
    glColor3f(0.1f, 0.6f, 0.1f);
    renderText(15.0f, LOGICAL_HEIGHT - 25.0f, GLUT_BITMAP_HELVETICA_18, "EDITOR DE VERTICES: ACTIVADO");
    
    glColor3f(0.2f, 0.2f, 0.2f);
    renderText(15.0f, LOGICAL_HEIGHT - 45.0f, GLUT_BITMAP_HELVETICA_12, "- Click izquierdo libre  : Insertar nuevo vertice");
    renderText(15.0f, LOGICAL_HEIGHT - 65.0f, GLUT_BITMAP_HELVETICA_12, "- Arrastrar vertice      : Mover posicion");
    renderText(15.0f, LOGICAL_HEIGHT - 90.0f, GLUT_BITMAP_HELVETICA_12, "[Presiona la tecla V para salir de este modo]");
  } else {
    // Modo Edición de Vértices DESACTIVADO: Mostrar la UI original de curvas
    glColor3f(0.0f, 0.0f, 0.0f);
    std::string editModeText = "Edit Mode";
    std::string drawModeText = "Draw Mode";
    std::string pointsText = "+/- to add/remove Control Points";
    std::string drawText = "d to draw Points";
    if (currentState == STATE_IDLE) {
      renderText(900.0f, LOGICAL_HEIGHT - 20.0f, GLUT_BITMAP_HELVETICA_18,
                 editModeText.c_str());
    } else {
      renderText(900.0f, LOGICAL_HEIGHT - 20.0f, GLUT_BITMAP_HELVETICA_18,
                 drawModeText.c_str());
    }
    renderText(10.0f, LOGICAL_HEIGHT - 20.0f, GLUT_BITMAP_HELVETICA_12,
               pointsText.c_str());
    renderText(10.0f, LOGICAL_HEIGHT - 40.0f, GLUT_BITMAP_HELVETICA_12,
               drawText.c_str());

    // Sugerencia sutil en la parte inferior para activar el modo
    glColor3f(0.5f, 0.5f, 0.5f);
    renderText(10.0f, LOGICAL_HEIGHT - 65.0f, GLUT_BITMAP_HELVETICA_12,
               "[Presiona la tecla V para ingresar al Editor de Vertices]");

    // Curve Selection UI
    if (activeCurveIndex > -1 && activeCurveIndex <= allCurves.size()) {
      std::string controlPointsText =
          "Control Points: " +
          std::to_string(allCurves[activeCurveIndex].controlPoints.size() - 2);
      renderText(LOGICAL_WIDTH - 200.0f, 20.0f, GLUT_BITMAP_HELVETICA_18,
                 controlPointsText.c_str());
      std::string resolutionText =
          "Curve Resolution: " +
          std::to_string(allCurves[activeCurveIndex].resolution);
      renderText(LOGICAL_WIDTH - 200.0f, 40.0f, GLUT_BITMAP_HELVETICA_18,
                 resolutionText.c_str());
    }
  }
}

// Function that draws 3D scene
void handleDrawScene3D(void) {
  gluPerspective(45.0, (float)winWidth / winHeight, 1.0, 50000.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  gluLookAt(500.0, 800.0, camera.zoom, 500.0, 300.0, 0.0, 0.0, 1.0, 0.0);

  // Translate the object
  glTranslatef(camera.panX, camera.panY, 0.0f);
  // Rotation
  float centerX = 425.0f;
  float centerY = 400.0f;
  float centerZ = 225.0f;
  glTranslatef(centerX, centerY, centerZ);
  glRotatef(camera.xRotation, 1.0, 0.0, 0.0);
  glRotatef(camera.yRotation, 0.0, 1.0, 0.0);
  glRotatef(camera.zRotation, 0.0, 0.0, 1.0);
  glTranslatef(-centerX, -centerY, -centerZ);

  // Capture OpenGL matrices from the last frame.
  glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetIntegerv(GL_VIEWPORT, viewport);

  // Draw Bezier Surface
  drawBezierSurface(allSurfaces);
}

// Function that draws all the scenes combined
void drawScene(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  if (currentMode == MODE_CURVE_2D) {
    handleDrawScene2D();
  } else {
    handleDrawScene3D();
  }

  glutSwapBuffers();
}

// Function that loads and renders a flat 3D 4x4 grid surface
void drawFlat3DSurface(float startX, float startY, float startZ) {
  BezierSurface newSurface;
  float spacing = 100.0f;  // Distance between control points

  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      float x = startX + (j * spacing);
      float y = startY;
      float z = startZ + (i * spacing);

      newSurface.controlPoints.push_back(
          Point(x, y, z, 1.0f, 0.0f, 0.0f, pointSize3D));
    }
  }
  allSurfaces.push_back(newSurface);
}

// Generic function that loads and renders 3D objects defined by Bezier Patches
template <size_t Rows, size_t Cols>
void drawObject3D(const float vertices[][3], const int (&patches)[Rows][Cols],
                  int resolution, float scale, float offsetX, float offsetY,
                  float offsetZ) {
  for (int i = 0; i < Rows; ++i) {
    BezierSurface patch;
    patch.resolution = resolution;

    for (int j = 0; j < Cols; ++j) {
      int rawIndex = patches[i][j];
      int verticesIndex = rawIndex - 1;

      if (verticesIndex < 0) {
        verticesIndex = rawIndex;
      }

      // Coordinates of patch
      float rawX = vertices[verticesIndex][0];
      float rawY = vertices[verticesIndex][1];
      float rawZ = vertices[verticesIndex][2];

      float finalX = (rawX * scale) + offsetX;
      float finalY = (rawZ * scale) + offsetY;
      float finalZ = (rawY * scale) + offsetZ;

      patch.controlPoints.push_back(
          Point(finalX, finalY, finalZ, 1.0f, 0.0f, 0.0f, pointSize3D));
    }
    allSurfaces.push_back(patch);
  }
}

// Function that converts 2D mouse coordinates into a 3D Ray in World Space.
void getRay(int mouseX, int mouseY, Point &nearPoint, Point &farPoint) {
  GLdouble objX, objY, objZ;

  // OpenGL origin is Bottom-Left, but Mouse origin is Top-Left.
  GLdouble winY = (double)viewport[3] - (double)mouseY;

  // Unproject the point at Z = 0.0 (Near Plane)
  gluUnProject(mouseX, winY, 0.0, model_view, projection, viewport, &objX,
               &objY, &objZ);
  nearPoint = Point((float)objX, (float)objY, (float)objZ);

  // Unproject the point at Z = 1.0 (Far Plane)
  gluUnProject(mouseX, winY, 1.0, model_view, projection, viewport, &objX,
               &objY, &objZ);
  farPoint = Point((float)objX, (float)objY, (float)objZ);
}

// Function that returns the index of the active point in 3D mode.
GLint getActivePoint3D(std::vector<Point> &pts, int mouseX, int mouseY) {
  Point nearPoint, farPoint;
  getRay(mouseX, mouseY, nearPoint, farPoint);

  float minThreshold = 1000.0f;
  int foundIndex = -1;

  for (size_t i = 0; i < pts.size(); ++i) {
    // Calculate distance from the Point[i] to the Mouse Ray
    float pointToLineDist = distPointToLine(pts[i], nearPoint, farPoint);

    // Pick the point if close enough
    if (pointToLineDist < 1500.0f) {
      if (pointToLineDist < minThreshold) {
        minThreshold = pointToLineDist;
        foundIndex = i;
      }
    }
  }
  return foundIndex;
}

void handleSelection2D(int xMouse, int yMouse) {
  // Select a Control Point (Dragging)
  bool foundPoint = false;

  for (size_t i = 0; i < allCurves.size(); ++i) {
    int hitIndex =
        getActivePoint2D(allCurves[i].controlPoints, 7.0, xMouse, yMouse);
    if (hitIndex != -1) {
      // activeCurveIndex = i;
      pointDragged = hitIndex;
      foundPoint = true;

      // Handle overlapping Points and add them to draggedPoints
      Point referenceClickedPoint = allCurves[i].controlPoints[hitIndex];
      collectOverlappingPoints(referenceClickedPoint, allCurves, draggedPoints);
    }
  }

  // Select a Line
  if (!foundPoint) {
    pointDragged = -1;
    activeCurveIndex = -1;
    bool foundLine = false;

    for (size_t i = 0; i < allCurves.size(); ++i) {
      if (isMouseOnCurve(allCurves[i], xMouse, yMouse)) {
        activeCurveIndex = i;
        foundLine = true;
        for (auto &curve : allCurves) {
          curve.isSelected = false;
        }
        allCurves[i].isSelected = true;
        break;
      }
    }

    // Deselect all
    if (!foundLine) {
      activeCurveIndex = -1;
      for (auto &curve : allCurves) {
        curve.isSelected = false;
      }
    }
  }
}

void handleDraw2D(int xMouse, int yMouse) {
  if (currentState == STATE_DRAWING_READY) {
    tempLineStartPoint = getSnappedPoint2D(allCurves, xMouse, yMouse, 1.0f,
                                           0.0f, 0.0f, pointSize2D);
    currentState = STATE_DRAWING_ACTIVE;
  } else if (currentState == STATE_DRAWING_ACTIVE) {
    // Finish drawing A Line
    Point endPoint = getSnappedPoint2D(allCurves, xMouse, yMouse, 1.0f, 0.0f,
                                       0.0f, pointSize2D);

    BezierCurve newCurve;
    newCurve.controlPoints.push_back(tempLineStartPoint);
    newCurve.controlPoints.push_back(endPoint);

    allCurves.push_back(newCurve);
    currentState = STATE_DRAWING_READY;
  }
}

void handleSelection3D(int xMouse, int yMouse) {
  // Select a point
  bool foundPoint = false;

  for (size_t i = 0; i < allSurfaces.size(); ++i) {
    int hitIndex =
        getActivePoint3D(allSurfaces[i].controlPoints, xMouse, yMouse);

    if (hitIndex != -1) {
      activeSurfaceIndex = i;
      pointDragged = hitIndex;
      foundPoint = true;

      for (auto &surface : allSurfaces) {
        surface.isSelected = false;
      }
      allSurfaces[i].isSelected = true;
      break;
    }
  }

  if (!foundPoint) {
    pointDragged = -1;
    activeSurfaceIndex = -1;
    for (auto &surface : allSurfaces) {
      surface.isSelected = false;
    }
  }
}

void mouseControl(int button, int state, int xMouse, int yMouse) {
  // Convert Mouse to World position
  Point worldPosition = getMouseWorldPosition(
      xMouse, yMouse, winWidth, winHeight, LOGICAL_WIDTH, LOGICAL_HEIGHT);
  float worldX = worldPosition.getX();
  float worldY = worldPosition.getY();

  if (currentMode == MODE_CURVE_2D && vertexEditEnabled) {
    if (button == GLUT_LEFT_BUTTON) {
      if (state == GLUT_DOWN) {
        int index = findNearestVertex(worldX, worldY);
        if (index != -1) {
          startMoveVertex(index);
        } else {
          insertVertex(worldX, worldY);
        }
      } else if (state == GLUT_UP) {
        stopMoveVertex();
      }
    }
    glutPostRedisplay();
    return;
  }

  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    if (currentMode == MODE_CURVE_2D) {
      if (currentState == STATE_IDLE) {
        handleSelection2D((int)worldX, (int)worldY);
      } else {
        handleDraw2D((int)worldX, (int)worldY);
      }
    }
  }

  if (currentMode == MODE_SURFACE_3D) {
    handleSelection3D(xMouse, yMouse);
  }

  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
    pointDragged = -1;
    draggedPoints.clear();
  }
  glutPostRedisplay();
}

void mouseMotion(GLint xMouse, GLint yMouse) {
  // Convert Mouse to World position
  Point worldPosition = getMouseWorldPosition(
      xMouse, yMouse, winWidth, winHeight, LOGICAL_WIDTH, LOGICAL_HEIGHT);
  float worldX = worldPosition.getX();
  float worldY = worldPosition.getY();

  if (currentMode == MODE_CURVE_2D && vertexEditEnabled) {
    if (selectedVertexIndex != -1) {
      moveSelectedVertex(worldX, worldY);
      glutPostRedisplay();
    }
    return;
  }

  if (pointDragged < 0) {
    return;
  }

  if (currentMode == MODE_CURVE_2D) {
    if (!draggedPoints.empty() && currentState == STATE_IDLE) {
      float newX = std::max(0.0f, std::min(worldX, LOGICAL_WIDTH));
      float newY = std::max(0.0f, std::min(worldY, LOGICAL_HEIGHT));

      for (auto &p : draggedPoints) {
        int curveIndex = p.first;
        int pointIndex = p.second;
        if (curveIndex >= 0 && curveIndex < allCurves.size()) {
          if (pointIndex >= 0 &&
              pointIndex < allCurves[curveIndex].controlPoints.size()) {
            allCurves[curveIndex].controlPoints[pointIndex].setX(newX);
            allCurves[curveIndex].controlPoints[pointIndex].setY(newY);
          }
        }
      }
    }
  }
  if (currentMode == MODE_SURFACE_3D) {
    // Get the mouse ray
    Point nearPoint, farPoint;
    getRay(xMouse, yMouse, nearPoint, farPoint);

    Point &targetPoint3D =
        allSurfaces[activeSurfaceIndex].controlPoints[pointDragged];

    // Move the targetPoint3D
    float dirZ = farPoint.getZ() - nearPoint.getZ();

    if (fabs(dirZ) > 0.001f) {
      float t = (targetPoint3D.getZ() - nearPoint.getZ()) / dirZ;

      float newX = nearPoint.getX() + (farPoint.getX() - nearPoint.getX()) * t;
      float newY = nearPoint.getY() + (farPoint.getY() - nearPoint.getY()) * t;

      targetPoint3D.setX(newX);
      targetPoint3D.setY(newY);
    }
  }

  glutPostRedisplay();
}

void createMenus(void) {
  // 2D menu options
  menuId2D = glutCreateMenu(menuHandler);
  glutAddMenuEntry("Toggle Grid", MENU_TOGGLE_GRID);
  glutAddMenuEntry("Toggle Draw/Edit Mode", MENU_TOGGLE_DRAW_MODE);
  glutAddMenuEntry("Toggle Points", MENU_TOGGLE_POINTS);
  glutAddMenuEntry("Toggle Control Polygon", MENU_TOGGLE_CONTROL_POLYGON);
  glutAddMenuEntry("Switch 3D", MENU_SWITCH_MODE);
  glutAddMenuEntry("Clear", MENU_CLEAR);
  glutAddMenuEntry("Quit", MENU_QUIT);

  // 3D menu options
  submenuIdObjects = glutCreateMenu(menuHandler);
  glutAddMenuEntry("Flat Surface", MENU_DRAW_SURFACE);
  glutAddMenuEntry("Teapot", MENU_DRAW_TEAPOT);
  glutAddMenuEntry("Rocket", MENU_DRAW_ROCKET);

  menuId3D = glutCreateMenu(menuHandler);
  glutAddSubMenu("Draw Objects", submenuIdObjects);
  glutAddMenuEntry("Toggle Points", MENU_TOGGLE_POINTS);
  glutAddMenuEntry("Toggle Control Polygon", MENU_TOGGLE_CONTROL_POLYGON);
  glutAddMenuEntry("Reset Camera", MENU_RESET_CAMERA);
  glutAddMenuEntry("Switch 2D", MENU_SWITCH_MODE);
  glutAddMenuEntry("Clear", MENU_CLEAR);
  glutAddMenuEntry("Quit", MENU_QUIT);
}

// Function that updates menu options for 2D/3D modes
void updateMenuForMode(void) {
  if (currentMode == MODE_CURVE_2D) {
    glutSetMenu(menuId2D);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
  } else {
    glutSetMenu(menuId3D);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
  }
}

// Function that handles menu options actions
void menuHandler(int value) {
  switch (value) {
    case MENU_TOGGLE_GRID:
      isGrid = !isGrid;
      break;
    case MENU_TOGGLE_POINTS:
      isDrawPoint = !isDrawPoint;
      break;
    case MENU_TOGGLE_CONTROL_POLYGON:
      isControlPolygon = !isControlPolygon;
      break;
    case MENU_TOGGLE_DRAW_MODE:
      if (currentMode == MODE_CURVE_2D) {
        if (currentState == STATE_IDLE) {
          currentState = STATE_DRAWING_READY;
        } else {
          currentState = STATE_IDLE;
        }
      }
      break;
    case MENU_SWITCH_MODE:
      if (currentMode == MODE_CURVE_2D) {
        currentMode = MODE_SURFACE_3D;
      } else {
        currentMode = MODE_CURVE_2D;
      }
      updateMenuForMode();
      break;
    case MENU_CLEAR:
      if (currentMode == MODE_CURVE_2D) {
        if (vertexEditEnabled) {
          vertices.clear();
          selectedVertexIndex = -1;
          std::cout << "Vertices limpiados." << std::endl;
        } else {
          allCurves.clear();
        }
      } else {
        allSurfaces.clear();
      }
      break;
    case MENU_RESET_CAMERA:
      camera.reset();
      break;
    case MENU_QUIT:
      exit(0);
      break;
    case MENU_DRAW_SURFACE:
      allSurfaces.clear();
      camera.reset();
      drawFlat3DSurface(200, 400, 0);
      break;
    case MENU_DRAW_TEAPOT:
      allSurfaces.clear();
      camera.reset();
      drawObject3D(teapotVertices, teapotPatches, 10, 80.0f, 425.0f, 300.0f,
                   225.0f);
      break;
    case MENU_DRAW_ROCKET:
      allSurfaces.clear();
      camera.reset();
      drawObject3D(rocketVertices, rocketPatches, 10, 150.0f, 425.0f, 300.0f,
                   0.0f);
      break;
    default:
      break;
  }
  glutPostRedisplay();
}

void setup(void) {
  glClearColor(1.0, 1.0, 1.0, 0.0);
  createMenus();
  updateMenuForMode();
}

void resizeWindow(int width, int height) {
  glViewport(0, 0, width, height);

  winWidth = width;
  winHeight = height;
}

// Function that increases the degree of the Bezier Curve without changing its
// shape
std::vector<Point> elevateDegree(std::vector<Point> oldPoints) {
  std::vector<Point> newPoints;
  int n = oldPoints.size() - 1;  // Current degree
  int newDegree = n + 1;         // New degree

  newPoints.push_back(oldPoints[0]);

  // Calculate intermediate points using Linear Interpolation
  for (int i = 1; i <= n; ++i) {
    float alpha = (float)i / (float)newDegree;

    float newX =
        alpha * oldPoints[i - 1].getX() + (1.0f - alpha) * oldPoints[i].getX();
    float newY =
        alpha * oldPoints[i - 1].getY() + (1.0f - alpha) * oldPoints[i].getY();

    newPoints.push_back(Point(newX, newY, 0.0f, 0.0f, 1.0f, 0.0f, pointSize2D));
  }

  newPoints.push_back(oldPoints[n]);

  return newPoints;
}

// Function that modifies the number of control points on the selected curve
void changeControlPointCount(int delta) {
  if (activeCurveIndex < 0 || activeCurveIndex >= allCurves.size()) {
    return;
  }

  BezierCurve &activeCurve = allCurves[activeCurveIndex];

  if (delta > 0) {
    activeCurve.controlPoints = elevateDegree(activeCurve.controlPoints);
  } else if (delta < 0) {
    if (activeCurve.controlPoints.size() > 2) {
      activeCurve.controlPoints.erase(activeCurve.controlPoints.end() - 2);
    }
  }
}

// Function that modifes the resolution of the Bezier Curve in 2D mode
void changeCurveResolution(int delta, int lowerBound) {
  if (currentMode != MODE_CURVE_2D || activeCurveIndex < 0 ||
      activeCurveIndex >= allCurves.size()) {
    return;
  }

  BezierCurve &activeCurve = allCurves[activeCurveIndex];

  activeCurve.resolution = std::max(lowerBound, activeCurve.resolution + delta);
}

void keyInput(unsigned char key, int x, int y) {
  switch (key) {
    case 27:
      exit(0);
      break;
    case 'v':
    case 'V':
      toggleVertexEditMode();
      break;
    case '+':
      changeControlPointCount(1);
      break;
    case '-':
      changeControlPointCount(-1);
      break;
    case ' ':
      if (currentMode == MODE_CURVE_2D) {
        currentMode = MODE_SURFACE_3D;
        updateMenuForMode();
      } else {
        currentMode = MODE_CURVE_2D;
        updateMenuForMode();
      }
      break;
    case 'd':
      if (currentMode == MODE_CURVE_2D) {
        if (currentState == STATE_IDLE) {
          currentState = STATE_DRAWING_READY;
        } else {
          currentState = STATE_IDLE;
        }
      }
      break;
    case 'c':
      isDrawPoint = !isDrawPoint;
      break;
    case 'x':
      if (currentMode == MODE_SURFACE_3D) {
        camera.xRotation += 5.0;
        if (camera.xRotation > 360.0) {
          camera.xRotation -= 360.0;
        }
      }
      break;
    case 'X':
      if (currentMode == MODE_SURFACE_3D) {
        camera.xRotation -= 5.0;
        if (camera.xRotation < 0.0) {
          camera.xRotation += 360.0;
        }
      }
      break;
    case 'y':
      if (currentMode == MODE_SURFACE_3D) {
        camera.yRotation += 5.0;
        if (camera.yRotation > 360.0) {
          camera.yRotation -= 360.0;
        }
      }
      break;
    case 'Y':
      if (currentMode == MODE_SURFACE_3D) {
        camera.yRotation -= 5.0;
        if (camera.yRotation < 0.0) {
          camera.yRotation += 360.0;
        }
      }
      break;
    case 'z':
      if (currentMode == MODE_SURFACE_3D) {
        camera.zRotation += 5.0;
        if (camera.zRotation > 360.0) {
          camera.zRotation -= 360.0;
        }
      }
      break;
    case 'Z':
      if (currentMode == MODE_SURFACE_3D) {
        camera.zRotation -= 5.0;
        if (camera.zRotation < 0.0) {
          camera.zRotation += 360.0;
        }
      }
      break;
    case 'i':
      if (currentMode == MODE_SURFACE_3D) {
        camera.zoom -= 50.0f;
        if (camera.zoom < 200.0f) {
          camera.zoom = 200.0f;
        }
      }
      break;
    case 'o':
      if (currentMode == MODE_SURFACE_3D) {
        camera.zoom += 50.0f;
      }
      break;
    case 'p':
      isControlPolygon = !isControlPolygon;
      break;
    case 'n':
      if (currentMode == MODE_SURFACE_3D) {
        allSurfaces.clear();
        camera.reset();
        drawFlat3DSurface(200, 400, 0);
      }
      break;
    case 'r':
      camera.reset();
      break;
    default:
      break;
  }
  glutPostRedisplay();
}

void specialKeyInput(int key, int x, int y) {
  switch (key) {
    case GLUT_KEY_UP:
      camera.panY += 10.0f;
      break;
    case GLUT_KEY_DOWN:
      camera.panY -= 10.0f;
      break;
    case GLUT_KEY_LEFT:
      camera.panX -= 10.0f;
      break;
    case GLUT_KEY_RIGHT:
      camera.panX += 10.0f;
      break;
    case GLUT_KEY_PAGE_UP:
      changeCurveResolution(5, 5);
      break;
    case GLUT_KEY_PAGE_DOWN:
      changeCurveResolution(-5, 5);
      break;
    default:
      break;
  }

  glutPostRedisplay();
}

int main(int argc, char **argv) {
  printInteraction();
  glutInit(&argc, argv);
  glutInitContextVersion(4, 3);
  glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize(1000, 1000);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Bezier Curves");
  glutDisplayFunc(drawScene);
  glutReshapeFunc(resizeWindow);
  glutKeyboardFunc(keyInput);
  glutSpecialFunc(specialKeyInput);
  glutMouseFunc(mouseControl);
  glutMotionFunc(mouseMotion);

  setup();

  glutMainLoop();
}
