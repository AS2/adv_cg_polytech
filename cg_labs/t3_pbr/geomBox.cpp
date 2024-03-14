#include <cmath>
#include "geomBox.h"

GeomBox::GeomBox(float xCenter, float yCenter, float zCenter, float scale, float xAngel, float yAngel, float zAngel) {
  vertices = std::vector<BoxVertex>({
    {{-1, -1,  1}, {0, -1, 0}, {1, 0, 0}},
    {{ 1, -1,  1}, {0, -1, 0}, {1, 0, 0}},
    {{ 1, -1, -1}, {0, -1, 0}, {1, 0, 0}},
    {{-1, -1, -1}, {0, -1, 0}, {1, 0, 0}},

    {{-1,  1, -1}, {0, 1, 0}, {1, 0, 0}},
    {{ 1,  1, -1}, {0, 1, 0}, {1, 0, 0}},
    {{ 1,  1,  1}, {0, 1, 0}, {1, 0, 0}},
    {{-1,  1,  1}, {0, 1, 0}, {1, 0, 0}},

    {{ 1, -1, -1}, {1, 0, 0}, {0, 0, 1}},
    {{ 1, -1,  1}, {1, 0, 0}, {0, 0, 1}},
    {{ 1,  1,  1}, {1, 0, 0}, {0, 0, 1}},
    {{ 1,  1, -1}, {1, 0, 0}, {0, 0, 1}},

    {{-1, -1,  1}, {-1, 0, 0}, {0, 0, -1}},
    {{-1, -1, -1}, {-1, 0, 0}, {0, 0, -1}},
    {{-1,  1, -1}, {-1, 0, 0}, {0, 0, -1}},
    {{-1,  1,  1}, {-1, 0, 0}, {0, 0, -1}},

    {{ 1, -1,  1}, {0, 0, 1}, {-1, 0, 0}},
    {{-1, -1,  1}, {0, 0, 1}, {-1, 0, 0}},
    {{-1,  1,  1}, {0, 0, 1}, {-1, 0, 0}},
    {{ 1,  1,  1}, {0, 0, 1}, {-1, 0, 0}},

    {{-1, -1, -1}, {0, 0, -1}, {1, 0, 0}},
    {{ 1, -1, -1}, {0, 0, -1}, {1, 0, 0}},
    {{ 1,  1, -1}, {0, 0, -1}, {1, 0, 0}},
    {{-1,  1, -1}, {0, 0, -1}, {1, 0, 0}}
  });

  indices = std::vector<unsigned short>({
        0, 2, 1, 0, 3, 2,
        4, 6, 5, 4, 7, 6,
        8, 10, 9, 8, 11, 10,
        12, 14, 13, 12, 15, 14,
        16, 18, 17, 16, 19, 18,
        20, 22, 21, 20, 23, 22
  });

  Scale(scale);
  Rotate(xAngel, yAngel, zAngel);
  Replace(xCenter, yCenter, zCenter);
}

void GeomBox::Scale(float scale) {
  scale = std::abs(scale);

  for (size_t i = 0; i < vertices.size(); i++) {
    vertices[i].pos.x *= scale;
    vertices[i].pos.y *= scale;
    vertices[i].pos.z *= scale;
  }
}

void GeomBox::Replace(float x, float y, float z) noexcept {
  boxCenterX += x;
  boxCenterY += y;
  boxCenterZ += z;

  for (size_t i = 0; i < vertices.size(); i++) {
    vertices[i].pos.x += x;
    vertices[i].pos.y += y;
    vertices[i].pos.z += z;
  }
}

// TODO: Rotation obj
void GeomBox::Rotate(float xAngel, float yAngel, float zAngel) noexcept {
}

GeomBox::~GeomBox() {
}
