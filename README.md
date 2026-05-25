# Interactive Bezier Curves & Surfaces

An interactive, real-time 3D and 2D modeling tool built with C++, OpenGL, and GLUT. This project implements De Casteljau's algorithm to render and manipulate Bezier curves and bicubic Tensor-Product surfaces from scratch.

## Table of Contents
* [Demos](#-demos)
  * [2D Surface](#2d-surface)
  * [3D 4x4 Mesh](#3d-4x4-mesh)
  * [3D Utah Teapot](#3d-utah-teapot)
* [Features](#-features)
  * [2D Curve Editor](#2d-curve-editor)
  * [3D Surface Rendering](#3d-surface-rendering)
* [Prerequisites & Dependencies](#️-prerequisites--dependencies)
* [Controls & Interaction](#-controls--interaction)
  * [Global Controls](#global-controls)
  * [2D Mode Specific](#2d-mode-specific)
  * [3D Mode Specific](#3d-mode-specific)
* [Architecture Overview](#-architecture-overview)


## Demos

### 2D Surface

### 3D 4x4 Mesh

### 3D Utah Teapot

## Features

### 2D Curve Editor
* **Draw & Edit Modes:** Dynamically draw new curves by clicking, and snap lines to existing control points to create continuous paths.
* **Degree Elevation:** Mathematically add control points (`+` key) to an existing curve without altering its shape, or destructively simplify the curve by removing points (`-` key).
* **Dynamic Resolution:** Increase or decrease the rendering resolution (number of line segments) in real-time.
* **Coincident Point Dragging:** Points overlapping in space are linked and drag together automatically.

### 3D Surface Rendering
* **Tensor-Product Surfaces:** Evaluates 3D surfaces as a "curve of curves" using generic, dynamic-sized control grids.
* **Procedural Generation:** Spawns flat 4x4 control meshes ready for user manipulation.
* **Classic Datasets:** Includes full support for rendering the standard **Utah Teapot** and a custom **Rocket** object directly from raw patch indices and vertex data.
* **Ray Picking:** Implements 3D unprojection (mouse-to-world rays) to allow clicking and dragging 3D control points in perspective space.
* **Camera System:** Fully featured orbital camera with panning, zooming, and 3-axis rotation.

## Prerequisites & Dependencies
To build and run this project, you will need:
* **C++ Compiler** (GCC, Clang, or MSVC)
* **OpenGL** (Included with most OS graphics drivers)
* **FreeGLUT** (Windowing and inputs)
* **GLEW** (OpenGL Extension Wrangler)

## Controls & Interaction

Right-click anywhere on the screen to access the **Contextual Menu** (Options change depending on whether you are in 2D or 3D mode).

### Global Controls
| Input | Action |
| :--- | :--- |
| **Left Click** | Select curves, drag control points, or draw new points. |
| **Right Click** | Open contextual tools menu. |
| **Spacebar** | Toggle between 2D Curve Mode and 3D Surface Mode. |
| **C** | Toggle visibility of Control Points. |
| **P** | Toggle visibility of the Control Polygon (Mesh cage). |
| **ESC** | Quit application. |

### 2D Mode Specific
| Input | Action |
| :--- | :--- |
| **D** | Toggle between Draw Mode and Edit Mode. |
| **+ / -** | Increase / Decrease the degree (control point count) of the selected curve. |
| **Page Up / Down** | Increase / Decrease the line resolution of the selected curve. |

### 3D Mode Specific
| Input | Action |
| :--- | :--- |
| **Arrow Keys** | Pan the camera (Up, Down, Left, Right). |
| **I / O** | Zoom camera In / Out. |
| **x / X** | Rotate camera on the X-Axis. |
| **y / Y** | Rotate camera on the Y-Axis. |
| **z / Z** | Rotate camera on the Z-Axis. |
| **R** | Reset camera to default position. |
| **N** | Spawn a new flat surface. |

## Architecture Overview
* `main.cpp`: Entry point, GLUT initialization, rendering loop, and input handling.
* `BezierMath.h`: Core mathematical implementation of Linear Interpolation and De Casteljau's algorithm for curves and grids.
* `Camera.h`: Struct managing 3D viewport state.
* `Utils.h`: Helper math for Ray-picking, vector projection, and coordinate mapping.
* `Point.h`: Data structure representing a 3D coordinate with color and size properties.
* `TeapotData.h` & `RocketData.h`: Stored arrays of vertices and indices for complex 3D patch models.
