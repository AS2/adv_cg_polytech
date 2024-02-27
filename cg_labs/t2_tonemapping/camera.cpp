#include "camera.h"

// Initialization camera method
HRESULT Camera::InitCamera() {
  phi = -XM_PIDIV4;
  theta = XM_PIDIV4;
  distanceToPoint = 4.0f;

  pointOfInterest = XMFLOAT3(0.0f, 0.0f, 0.0f);

  return S_OK;
}

// Update frame method
void Camera::Frame() {
  XMFLOAT3 pos = XMFLOAT3(
    cosf(theta) * cosf(phi), 
    sinf(theta), 
    cosf(theta) * sinf(phi)
  );

  pos.x = pos.x * distanceToPoint + pointOfInterest.x;
  pos.y = pos.y * distanceToPoint + pointOfInterest.y;
  pos.z = pos.z * distanceToPoint + pointOfInterest.z;
  
  float upTheta = theta + XM_PIDIV2;
  XMFLOAT3 up = XMFLOAT3(
    cosf(upTheta) * cosf(phi), 
    sinf(upTheta), 
    cosf(upTheta) * sinf(phi)
  );

  viewMatrix = DirectX::XMMatrixLookAtLH(
    DirectX::XMVectorSet(pos.x, pos.y, pos.z, 0.0f),
    DirectX::XMVectorSet(pointOfInterest.x, pointOfInterest.y, pointOfInterest.z, 0.0f),
    DirectX::XMVectorSet(up.x, up.y, up.z, 0.0f)
  );
}

void Camera::ProvideInput(const Input& input) {
  // handle camera rotations
  XMFLOAT3 mouseMove = input.IsMouseUsed();
  Rotate(mouseMove.x / MOVEMENT_DOWNSHIFTING, mouseMove.y / MOVEMENT_DOWNSHIFTING, mouseMove.z / MOVEMENT_DOWNSHIFTING);

  // handle camera moving
  float dx = 0, dz = 0;
  if (input.IsKeyPressed(DIK_W))
    dx += 10;
  if (input.IsKeyPressed(DIK_S))
    dx -= 10;
  if (input.IsKeyPressed(DIK_A))
    dz += 10;
  if (input.IsKeyPressed(DIK_D))
    dz -= 10;
  Move(dx / MOVEMENT_DOWNSHIFTING, 0, dz / MOVEMENT_DOWNSHIFTING);
}

void Camera::Rotate(float dx, float dy, float wheel) {
  phi -= dx;

  theta += dy;
  theta = min(max(theta, -XM_PIDIV2), XM_PIDIV2);

  distanceToPoint -= wheel;
  distanceToPoint = max(distanceToPoint, 1.0f);
}


void Camera::Move(float dx, float dy, float dz) {
    pointOfInterest.x += dx;
    pointOfInterest.y += dy;
    pointOfInterest.z += dz;
}

// Get view matrix method
void Camera::GetBaseViewMatrix(XMMATRIX& viewMatrix) {
  viewMatrix = this->viewMatrix;
};
