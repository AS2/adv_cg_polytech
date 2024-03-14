#pragma once

#include <vector>

using namespace DirectX;

class GeomSphere {
public:
  GeomSphere() {};

  GeomSphere(float radius, unsigned int LatLines = 10, unsigned intLongLines = 10,
    float xCenter = 0.0f, float yCenter = 0.0f, float zCenter = 0.0f) noexcept;

  void Scale(float scale) noexcept;

  void Replace(float x, float y, float z) noexcept;

  void Rotate(float xAngel, float yAngel, float zAngel) noexcept {};

  ~GeomSphere();
protected:
  // TODO: make uniform structure of vertexes for all geom primitives
  struct SphereVertex
  {
    XMFLOAT3 pos;          // positional coords
    //XMFLOAT3 norm;         // normal vec
    //XMFLOAT3 tangent;      // tangent vec
  };

  void GenerateSphere(unsigned int LatLines, unsigned int LongLines);

  // Sphere light geometry params
  unsigned int numSphereVertices = 0;
  unsigned int numSphereFaces = 0;
  float radius = 1.0f;
  float sphereCenterX = 0, sphereCenterY = 0, sphereCenterZ = 0;


  unsigned int vertices_cnt = 0;
  std::vector<SphereVertex> vertices;
  std::vector<unsigned int> indices;
};