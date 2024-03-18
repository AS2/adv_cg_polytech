#include "scene.h"

HRESULT Scene::Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
  int square_size = 11;
  HRESULT hr = S_OK;

  spheres.resize(square_size * square_size);
  for (int x = 0; x < square_size; x++)
    for (int y = 0; y < square_size; y++) {
      spheres[x * square_size + y] = Sphere(0.55f, XMFLOAT3(-5, -7.5f + 1.5f * x, -7.5f + 1.5f * y), 
        XMFLOAT3(0.1f * x, 0.1f * y, 0.025f), 0.1f * x, 0.1f * y, 15, 15);
      hr = spheres[x * square_size + y].Init(device, context, screenWidth, screenHeight);
      if (FAILED(hr))
        return hr;
    }

  // Init lights
  lights.reserve(1);
  lights.push_back(Light(XMFLOAT4(1.f, 1.f, 1.0f, 100.0f), 0.0f, 0.0f, 0.0f));
  
  hr = lights[0].Init(device, context, screenWidth, screenHeight);
  if (FAILED(hr))
    return hr;

  sb = Skybox(L"./src/skybox.dds", 30, 30);
  hr = sb.Init(device, context, screenWidth, screenHeight);
  if (FAILED(hr))
    return hr;

#ifdef _DEBUG
  hr = context->QueryInterface(__uuidof(pAnnotation), reinterpret_cast<void**>(&pAnnotation));
  if (FAILED(hr))
    return hr;
#endif

  return hr;
}

void Scene::ProvideInput(const Input& input) {
  for (auto& light : lights)
    light.ProvideInput(input);

  for (auto& sphere : spheres)
    sphere.ProvideInput(input);
}

void Scene::Release() {
#ifdef _DEBUG
  if (pAnnotation) pAnnotation->Release();
#endif
  sb.Release();

  for (auto& sphere : spheres)
    sphere.Release();
  
  for (auto& light : lights)
    light.Release();
}

void Scene::Render(ID3D11DeviceContext* context) {
  sb.Render(context);

#ifdef _DEBUG
  pAnnotation->BeginEvent((LPCWSTR)L"Draw Spheres");
  std::string indexBufferName = "Indexes buffer", vertexBufferName = "Vertexes buffer";
#endif
  for (auto& sphere : spheres)
    sphere.Render(context);
#ifdef _DEBUG
  pAnnotation->EndEvent();
#endif

  for (auto& light : lights)
    light.Render(context);
}

bool Scene::Update(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMVECTOR cameraPos) {
  sb.Update(context, viewMatrix, projectionMatrix, XMFLOAT3(XMVectorGetX(cameraPos), XMVectorGetY(cameraPos), XMVectorGetZ(cameraPos)));

  for (auto& light : lights)
    light.Update(context, viewMatrix, projectionMatrix, cameraPos);
  
  for (auto& sphere : spheres)
    sphere.Update(context, viewMatrix, projectionMatrix, cameraPos, lights);

  return true;
}

void Scene::Resize(int screenWidth, int screenHeight) {
  sb.Resize(screenWidth, screenHeight);
};
