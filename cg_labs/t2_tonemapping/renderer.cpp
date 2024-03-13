#include <string>

#include "renderer.h"

using namespace DirectX;

Renderer& Renderer::GetInstance() {
  static Renderer rendererInstance;
  return rendererInstance;
}

HRESULT Renderer::InitDevice(const HWND& hWnd) {
  HRESULT hr = S_OK;

  RECT rc;
  GetClientRect(hWnd, &rc);
  UINT width = rc.right - rc.left;
  UINT height = rc.bottom - rc.top;


  // Create debug layer with DEBUG
  UINT createDeviceFlags = 0;
#ifdef _DEBUG
  createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_DRIVER_TYPE driverTypes[] =
  {
      D3D_DRIVER_TYPE_HARDWARE,
      D3D_DRIVER_TYPE_WARP,
      D3D_DRIVER_TYPE_REFERENCE,
  };
  UINT numDriverTypes = ARRAYSIZE(driverTypes);

  D3D_FEATURE_LEVEL featureLevels[] =
  {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
  };
  UINT numFeatureLevels = ARRAYSIZE(featureLevels);

  for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
  {
    driverType = driverTypes[driverTypeIndex];
    hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
      D3D11_SDK_VERSION, &pd3dDevice, &featureLevel, &pImmediateContext);

    if (hr == E_INVALIDARG)
    {
      // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
      hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
        D3D11_SDK_VERSION, &pd3dDevice, &featureLevel, &pImmediateContext);
    }

    if (SUCCEEDED(hr))
      break;
  }

  if (FAILED(hr))
    return hr;

  // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
  IDXGIFactory1* dxgiFactory = nullptr;
  {
    IDXGIDevice* dxgiDevice = nullptr;
    hr = pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
    if (SUCCEEDED(hr))
    {
      IDXGIAdapter* adapter = nullptr;
      hr = dxgiDevice->GetAdapter(&adapter);
      if (SUCCEEDED(hr))
      {
        hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
        adapter->Release();
      }
      dxgiDevice->Release();
    }
  }
  if (FAILED(hr))
    return hr;

  // Create swap chain
  IDXGIFactory2* dxgiFactory2 = nullptr;
  hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
  
  if (dxgiFactory2) // <-- this check is equal to check on "SUCCESED(hr)", saw in documentation
  {
    // DirectX 11.1 or later
    hr = pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&pd3dDevice1));
    if (SUCCEEDED(hr))
    {
      (void)pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&pImmediateContext1));
    }

    DXGI_SWAP_CHAIN_DESC1 sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.Width = width;
    sd.Height = height;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    sd.BufferCount = 2;

    hr = dxgiFactory2->CreateSwapChainForHwnd(pd3dDevice, hWnd, &sd, nullptr, nullptr, &pSwapChain1);
    if (SUCCEEDED(hr))
    {
      hr = pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&pSwapChain));
    }

    dxgiFactory2->Release();
  }
  else
  {
    // DirectX 11.0 systems
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    sd.Windowed = TRUE;

    hr = dxgiFactory->CreateSwapChain(pd3dDevice, &sd, &pSwapChain);
  }

  // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
  dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

  dxgiFactory->Release();

  // Checks for >=11.1 and 11.0 versions
  if (FAILED(hr))
    return hr;

  // Create a render target view
  ID3D11Texture2D* pBackBuffer = nullptr;
  hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
  if (FAILED(hr))
    return hr;

  D3D11_VIEWPORT viewport;
  viewport.TopLeftX = 0;
  viewport.TopLeftY = 0;
  viewport.Width = (FLOAT)input.GetWidth();
  viewport.Height = (FLOAT)input.GetHeight();
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
  pImmediateContext->RSSetViewports(1, &viewport);

  D3D11_RECT rect;
  rect.left = 0;
  rect.top = 0;
  rect.right = input.GetWidth();
  rect.bottom = input.GetHeight();
  pImmediateContext->RSSetScissorRects(1, &rect);

  
  pRenderedSceneTexture = new RenderTargetTexture(width, height);
  hr = pRenderedSceneTexture->initResource(pd3dDevice, pImmediateContext);
  if (FAILED(hr))
    return hr;

  pPostProcessedTexture = new RenderTargetTexture(width, height);
  hr = pPostProcessedTexture->initResource(pd3dDevice, pImmediateContext, pBackBuffer);
  if (FAILED(hr))
    return hr;

  pBackBuffer->Release();
  return S_OK;
}

HRESULT Renderer::Init(const HWND& hWnd, const HINSTANCE& hInstance, UINT screenWidth, UINT screenHeight) {
  HRESULT hr = input.InitInputs(hInstance, hWnd, screenWidth, screenHeight);
  if (FAILED(hr))
    return hr;

  hr = camera.InitCamera();
  if (FAILED(hr))
    return hr;

  hr = InitDevice(hWnd);
  if (FAILED(hr))
    return hr;

  hr = sc.Init(pd3dDevice, pImmediateContext, screenWidth, screenHeight);
  if (FAILED(hr))
    return hr;

  hr = PP.Init(pd3dDevice, pImmediateContext);
  if (FAILED(hr))
    return hr;

#ifdef _DEBUG
  hr = pImmediateContext->QueryInterface(__uuidof(pAnnotation), reinterpret_cast<void**>(&pAnnotation));
  if (FAILED(hr))
    return hr;
#endif

  return S_OK;
}

void Renderer::HandleInput() {
  camera.ProvideInput(input);
  sc.ProvideInput(input);
}

// Update frame method
bool Renderer::Update() {
  // update inputs
  input.Update();
  
  // update camera
  HandleInput();
  camera.Update();
  PP.Update(pd3dDevice, pImmediateContext);

  // Get the view matrix
  XMMATRIX mView;
  camera.GetBaseViewMatrix(mView);
  // Get the projection matrix
  XMMATRIX mProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, (FLOAT)input.GetWidth() / (FLOAT)input.GetHeight(), 0.01f, 100.0f);
  
  HRESULT hr = sc.Update(pImmediateContext, mView, mProjection, camera.GetPos());
  if (FAILED(hr))
    return SUCCEEDED(hr);

  return SUCCEEDED(hr);
}

HRESULT Renderer::Render() {
  pImmediateContext->ClearState();
  ID3D11ShaderResourceView* nullSRV = nullptr;
  pImmediateContext->PSSetShaderResources(0, 1, &nullSRV);
  pRenderedSceneTexture->set(pd3dDevice, pImmediateContext);

#ifdef _DEBUG
  pAnnotation->BeginEvent((LPCWSTR)(L"Clear background"));
#endif
  pRenderedSceneTexture->clear(1.0f, 1.0f, 1.0f, pd3dDevice, pImmediateContext);
#ifdef _DEBUG
  pAnnotation->EndEvent();
#endif

  D3D11_VIEWPORT viewport;
  viewport.TopLeftX = 0;
  viewport.TopLeftY = 0;
  viewport.Width = (FLOAT)input.GetWidth();
  viewport.Height = (FLOAT)input.GetHeight();
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
  pImmediateContext->RSSetViewports(1, &viewport);

  D3D11_RECT rect;
  rect.left = 0;
  rect.top = 0;
  rect.right = input.GetWidth();
  rect.bottom = input.GetHeight();
  pImmediateContext->RSSetScissorRects(1, &rect);

  sc.Render(pImmediateContext);

  PP.applyTonemapEffect(pd3dDevice, pImmediateContext, pAnnotation, pRenderedSceneTexture, pPostProcessedTexture);

  return pSwapChain->Present(0, 0);
}

void Renderer::CleanupDevice() {
  PP.Release();
  camera.Release();
  input.Release();
  sc.Release();

#ifdef _DEBUG
  if (pAnnotation) pAnnotation->Release();
#endif
  if (pPostProcessedTexture)
    delete pPostProcessedTexture;

  if (pRenderedSceneTexture) 
    delete pRenderedSceneTexture;
  
  if (pSwapChain1) pSwapChain1->Release();
  if (pSwapChain) pSwapChain->Release();
  if (pImmediateContext1) pImmediateContext1->Release();
  if (pImmediateContext) pImmediateContext->Release();
  if (pd3dDevice1) pd3dDevice1->Release();
  
#ifdef _DEBUG
  if (pd3dDevice) {
    ID3D11Debug* d3dDebug = nullptr;
    pd3dDevice->QueryInterface(IID_PPV_ARGS(&d3dDebug));

    UINT references = pd3dDevice->Release();
    if (references > 1) {
      d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
    }
    d3dDebug->Release();
  }
#else
  if (pd3dDevice) pd3dDevice->Release();
#endif
}

HRESULT Renderer::ResizeWindow(const HWND& hWnd) {
  if (pSwapChain) {
    RECT rc;
    GetClientRect(hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    if ((width != input.GetWidth() || height != input.GetHeight())) {

      if (pRenderedSceneTexture)
        pRenderedSceneTexture->Release();

      if (pPostProcessedTexture)
        pPostProcessedTexture->Release();

      HRESULT hr = pSwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
      if (SUCCEEDED(hr)) {
        input.Resize(width, height);
        sc.Resize(width, height);
      }

      // Create a render target view
      ID3D11Texture2D* pBackBuffer = nullptr;
      hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
      if (FAILED(hr))
        return hr;
      
      if (pRenderedSceneTexture) {
        pRenderedSceneTexture->setScreenSize(width, height);
        hr = pRenderedSceneTexture->initResource(pd3dDevice, pImmediateContext);
        if (FAILED(hr))
          return hr;

        pPostProcessedTexture->setScreenSize(width, height);
        hr = pPostProcessedTexture->initResource(pd3dDevice, pImmediateContext, pBackBuffer);
        if (FAILED(hr))
          return hr;

        pBackBuffer->Release();
      }
      return SUCCEEDED(hr);
    }
  }
  return S_OK;
}
