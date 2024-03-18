#pragma once 

#include "materials.h"
#include "../libs/ImGUI/imgui.h"
#include "../libs/ImGUI/imgui_impl_dx11.h"
#include "../libs/ImGUI/imgui_impl_win32.h"

// Class to storage and manage PBR material
class ImGUIHandler {
public:
  static ImGUIHandler& GetInstance() {
    static ImGUIHandler instance;
    return instance;
  }


  void Init(HWND window,
    ID3D11Device* pDevice,
    ID3D11DeviceContext* pContext)
  {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    /*io.ConfigFlags |= io.WantCaptureMouse;
    io.ConfigFlags |= io.WantCaptureMouse;*/

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(pDevice, pContext);
  }

  void Render() {
    // Init new frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Generate window
    ImGui::Begin("PBR params");
    
    // Enum pbr mode
    ImGui::RadioButton("Full", pbrModeRef, static_cast<int>(PBRMode::allPBR));
    ImGui::RadioButton("Normal distribution", pbrModeRef, static_cast<int>(PBRMode::normal));
    ImGui::RadioButton("Geometry", pbrModeRef, static_cast<int>(PBRMode::geom));
    ImGui::RadioButton("Fresnel", pbrModeRef, static_cast<int>(PBRMode::fresnel));
    
    // PBR Materials params
    ImGui::SliderFloat("Albedo-R", &pbrMaterial.albedo.x, 0, 1);
    ImGui::SliderFloat("Albedo-G", &pbrMaterial.albedo.y, 0, 1);
    ImGui::SliderFloat("Albedo-B", &pbrMaterial.albedo.z, 0, 1);
    ImGui::SliderFloat("Roughness", &pbrMaterial.roughness, 0, 1);
    ImGui::SliderFloat("Metalness ", &pbrMaterial.metalness, 0, 1);
    
    ImGui::End();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  }

  void Release() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
  }

  PBRMaterial GetMaterial() {
    return pbrMaterial;
  }

  PBRMode GetMode() {
    return (PBRMode)(pbrMode);
  }

  ImGUIHandler(const ImGUIHandler&) = delete;
  ImGUIHandler(ImGUIHandler&&) = delete;
private:
  ImGUIHandler() {
    pbrModeRef = &pbrMode;
    pbrMode = (int)PBRMode::allPBR;
  };

  PBRMaterial pbrMaterial;
  int pbrMode;  // TODO : Change int to PBRMode enum class with reinterpret_cast
  int* pbrModeRef = nullptr;
};
