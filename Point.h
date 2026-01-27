#ifndef POINT_H
#define POINT_H

// clang-format off
#include <GL/glew.h>
#include <GL/freeglut.h>
// clang-format on
#include <cmath>
#include <math.h>

class Point {
public:
  Point(float x = 0.0f, float y = 0.0f, float z = 0.0f, float r = 0.0f,
        float g = 0.0f, float b = 0.0f, float size = 0.0f)
      : xVal(x), yVal(y), zVal(z), red(r), green(g), blue(b), sizeVal(size) {}

  void setColor(float red, float green, float blue) {
    red = red;
    green = green;
    blue = blue;
  }

  void setX(float x) { xVal = x; }
  void setY(float y) { yVal = y; }
  void setZ(float z) { zVal = z; }

  float getX() const { return xVal; }
  float getY() const { return yVal; }
  float getZ() const { return zVal; }

  void drawPoint2D() {
    glColor3f(red, green, blue);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(xVal, yVal, 0.0f);

    for (size_t i = 0; i <= 30; ++i) {
      float t = 2.0f * M_PI * float(i) / 30.0f;
      glVertex3f(xVal + sizeVal * cos(t), yVal + sizeVal * sin(t), 0.0);
    }
    glEnd();
  }

  void drawPoint3D() {
    glColor3f(red, green, blue);
    glPushMatrix();
    glTranslatef(xVal, yVal, zVal);
    glutSolidSphere(sizeVal, 10, 10);
    glPopMatrix();
  }

private:
  float xVal, yVal, zVal, red, green, blue, sizeVal;
};

#endif
