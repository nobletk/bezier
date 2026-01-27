#ifndef CAMERA_H
#define CAMERA_H

struct Camera {
  float xRotation;
  float yRotation;
  float zRotation;
  float zoom;
  float panX;
  float panY;

  Camera() { reset(); }

  void reset() {
    xRotation = 0.0f;
    yRotation = 0.0f;
    zRotation = 0.0f;
    zoom = 1800.0f;
    panX = 0.0f;
    panY = 0.0f;
  }
};

#endif
