#pragma once

#include <vector>

class GeomBox {
public:
  GeomBox(float xCenter = 0.0f, float yCenter = 0.0f, float zCenter = 0.0f, float scale = 1.0f, float xAngel = 0.0f, float yAngel = 0.0f, float zAngel = 0.0f);
  
  void Scale(float scale);

  void Replace(float x, float y, float z) noexcept;

  void Rotate(float xAngel, float yAngel, float zAngel) noexcept;

  ~GeomBox() noexcept;
protected:
  struct BoxVertex
  {
    struct {
      float x, y, z;
    } pos;       // positional coords
    struct {
      float x, y, z;
    }  normal;    // normal vec
    struct {
      float x, y, z;
    }  tangent;   // tangent vec
  };

  std::vector<BoxVertex> vertices;
  std::vector<unsigned short> indices;

  float boxCenterX = 0, boxCenterY = 0, boxCenterZ = 0;
};
