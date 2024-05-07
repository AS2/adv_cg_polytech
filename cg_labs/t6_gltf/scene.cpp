#include "scene.h"

HRESULT Scene::Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
  HRESULT hr = S_OK;

  // Init skybox
  sb = Skybox(L"./src/envs/env_1k_4.hdr", 30, 30);
  hr = sb.Init(device, context, screenWidth, screenHeight);
  if (FAILED(hr))
    return hr;
  maps = sb.GetMaps();

  // Init model
  model = Model("./src/models/rgo/scene.gltf", "./src/models/rgo/scene.bin", sb, PBRPoorMaterial(0.2, 0.3, 0.04));
  //model = Model("./src/models/Fallout 10mm/scene.gltf", "./src/models/Fallout 10mm/scene.bin", sb);
  hr = model.Init(device, context, screenWidth, screenHeight);
  if (FAILED(hr))
    return hr;

  // Init lights
  lights.reserve(1);
  lights.push_back(Light(XMFLOAT4(1.f, 0.0f, 0.0f, 0.3f), 2.0f, 2.0f, 2.0f));
  
  hr = lights[0].Init(device, context, screenWidth, screenHeight);
  if (FAILED(hr))
    return hr;

  return hr;
}

void Scene::ProvideInput(const Input& input) {
  for (auto& light : lights)
    light.ProvideInput(input);
}

void Scene::Release() {
  sb.Release();

  model.Release();
  
  for (auto& light : lights)
    light.Release();
}

void Scene::Render(ID3D11DeviceContext* context) {
  sb.Render(context);

  beginEvent(L"Drawing model");
  model.Render(context);
  endEvent();

  for (auto& light : lights)
    light.Render(context);
}

bool Scene::Update(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMVECTOR cameraPos) {
  sb.Update(context, viewMatrix, projectionMatrix, XMFLOAT3(XMVectorGetX(cameraPos), XMVectorGetY(cameraPos), XMVectorGetZ(cameraPos)));

  model.Update(context, viewMatrix, projectionMatrix, cameraPos, lights);

  for (auto& light : lights) {
    light.Update(context, viewMatrix, projectionMatrix, cameraPos);
    if (isOff)
      light.GetLightColorRef()->w = 0.0f;
    else
      light.GetLightColorRef()->w = intensity;
  }
  
  return true;
}

void Scene::Resize(int screenWidth, int screenHeight) {
  sb.Resize(screenWidth, screenHeight);
};

void Scene::RenderGUI() {
  // Generate window
  ImGui::Begin("Scene params");

  // TODO : make different options for viewing (regular, only normals and so on)
  ImGui::Text("View mode");
  /*ImGui::RadioButton("Full IBL", reinterpret_cast<int*>(&iblMode), static_cast<int>(IBLMode::full));
  ImGui::RadioButton("Diffuse only", reinterpret_cast<int*>(&iblMode), static_cast<int>(IBLMode::diffuse));
  ImGui::RadioButton("Specular only", reinterpret_cast<int*>(&iblMode), static_cast<int>(IBLMode::specular));
  ImGui::RadioButton("No IBL", reinterpret_cast<int*>(&iblMode), static_cast<int>(IBLMode::nothing));
  */
  ImGui::Text("Lights params");
  ImGui::Checkbox("Off light", &isOff);
  for (int i = 0; i < lights.size(); i++) {
    ImGui::Text((std::string("Light-") + std::to_string(i + 1)).c_str());
    ImGui::ColorEdit3("Color", &lights[i].GetLightColorRef()->x);
    ImGui::SliderFloat("Intensity", &intensity, 0, 10.0f);
    ImGui::SliderFloat("Pos-X", &lights[i].GetLightPositionRef()->x, -100.f, 100.f);
    ImGui::SliderFloat("Pos-Y", &lights[i].GetLightPositionRef()->y, -100.f, 100.f);
    ImGui::SliderFloat("Pos-Z", &lights[i].GetLightPositionRef()->z, -100.f, 100.f);
  }

  ImGui::End();
}
