#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <directxmath.h>

#include <ctime>

#include "camera.h"
#include "scene.h"
#include "input.h"

#include "renderTargetTexture.h"
#include "postprocessing.h"

// Make renderer class
class Renderer {
public:
  // Make class singleton
  static Renderer& GetInstance();
  Renderer(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;

  // Init Renderer method
  HRESULT Init(const HWND& hWnd, const HINSTANCE& g_hInstance, UINT screenWidth, UINT screenHeight);

  // Update frame method
  bool Update();

  // Scene render method
  HRESULT Render();

  // Device cleaner method
  void CleanupDevice();

  // Window resize method
  HRESULT ResizeWindow(const HWND& hWnd);

private:
  // Private constructor (for singleton)
  Renderer() = default;

  // Initialization device method
  HRESULT InitDevice(const HWND& hWnd);

  // Initialization device method
  HRESULT InitDepthBuffer();

  void HandleInput();

  // DirectX11 variables
  D3D_DRIVER_TYPE         driverType = D3D_DRIVER_TYPE_NULL;
  D3D_FEATURE_LEVEL       featureLevel = D3D_FEATURE_LEVEL_11_0;
  
  ID3D11Device*           pd3dDevice = nullptr;
  ID3D11Device1*          pd3dDevice1 = nullptr;
  
  ID3D11DeviceContext*    pImmediateContext = nullptr;
  ID3D11DeviceContext1*   pImmediateContext1 = nullptr;
  
  IDXGISwapChain*         pSwapChain = nullptr;
  IDXGISwapChain1*        pSwapChain1 = nullptr;

  ID3D11Texture2D* pDepthBuffer = nullptr;
  ID3D11DepthStencilView* pDepthBufferDSV = nullptr;

  RenderTargetTexture* pRenderedSceneTexture;
  RenderTargetTexture* pPostProcessedTexture;

  ID3DUserDefinedAnnotation* pAnnotation = nullptr;

  // initialization other things (camera, input devices, etc.)
  Camera camera;
  Scene sc;
  Input input;

  // initialization postprocessing pipeline
  Postprocessing PP;
};
