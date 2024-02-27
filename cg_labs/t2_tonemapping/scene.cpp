#include "scene.h"

HRESULT Scene::Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
  // Init box
  HRESULT hr = box.Init(device, context, screenWidth, screenHeight, {0.0f});
  if (FAILED(hr))
    return hr;

  // Init lights
  lights = std::vector<Light>(3);
  hr = lights[0].Init(device, context, screenWidth, screenHeight, 
    XMFLOAT4(1.f, 0.f, 0.0f, 1.0f), XMFLOAT4(0.f, 5.5f, 0.8f, 1.f));
  hr = lights[1].Init(device, context, screenWidth, screenHeight, 
    XMFLOAT4(0.f, 1.f, 0.0f, 5.0f), XMFLOAT4(0.8f, 5.5f, 0.f, 1.f));
  hr = lights[2].Init(device, context, screenWidth, screenHeight, 
    XMFLOAT4(0.f, 0.f, 1.0f, 10.0f), XMFLOAT4(0.0f, 5.5f, -0.8f, 1.f));
  /*hr = lights[3].Init(device, context, screenWidth, screenHeight,
    XMFLOAT4(1.f, 1.f, 1.0f, 1.f), XMFLOAT4(0.0f, 0.f, 0.f, 1.f));*/

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
}

void Scene::Release() {
#ifdef _DEBUG
  if (pAnnotation) pAnnotation->Release();
#endif

  box.Release();

  for (auto& light : lights)
    light.Release();
}

void Scene::Render(ID3D11DeviceContext* context) {
  
#ifdef _DEBUG
  pAnnotation->BeginEvent((LPCWSTR)L"Draw Cube");
  std::string indexBufferName = "Indexes buffer", vertexBufferName = "Vertexes buffer";
#endif
  box.Render(context);
#ifdef _DEBUG
  pAnnotation->EndEvent();
#endif

  // Comment because depth buffer isnt implementet yet!
  /*
  for (auto& light : lights)
    light.Render(context);
  */
 }

bool Scene::FrameBoxes(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMVECTOR cameraPos) {
  // Update world matrix angle of first cube
  auto duration = 1.0f;// Timer::GetInstance().Clock();
  XMMATRIX worldMatrix = XMMatrixIdentity(); //XMMatrixRotationY((float)duration * angle_velocity * 0.5f)*XMMatrixRotationZ((float)(sin(duration * angle_velocity * 0.30) * 0.25f));

  // Update world matrix angle of second cube
  box.Frame(context, worldMatrix, viewMatrix, projectionMatrix, cameraPos, lights);

  return true;
}


bool Scene::Frame(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMVECTOR cameraPos) {
  FrameBoxes(context, viewMatrix, projectionMatrix, cameraPos);
  
  for (auto& light : lights)
    light.Frame(context, viewMatrix, projectionMatrix, cameraPos);
  return true;
}

void Scene::Resize(int screenWidth, int screenHeight) {
  box.Resize(screenWidth, screenHeight);
  for (auto& light : lights)
    light.Resize(screenWidth, screenHeight);
};
