#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <directxmath.h>
#include <string>
#include <vector>

#include "input.h"
#include "light.h"
#include "Sphere.h"
#include "box.h"
#include "skybox.h"

using namespace DirectX;

class Scene {
public:
  HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight);

  void Release();

  void Resize(int screenWidth, int screenHeight);

  void Render(ID3D11DeviceContext* context);

  void ProvideInput(const Input &input);

  bool Update(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMVECTOR cameraPos);

private:
  
#ifdef _DEBUG
  ID3DUserDefinedAnnotation* pAnnotation = nullptr;
#endif

  std::vector<Sphere> spheres;
  std::vector<Light> lights;
  Skybox sb;
};
