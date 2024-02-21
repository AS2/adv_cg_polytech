#include <string>

#include "renderer.h"

using namespace DirectX;

Renderer& Renderer::GetInstance() {
  static Renderer rendererInstance;
  return rendererInstance;
}

HRESULT Renderer::CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
  HRESULT hr = S_OK;

  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
  // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
  // Setting this flag improves the shader debugging experience, but still allows 
  // the shaders to be optimized and to run exactly the way they will run in 
  // the release configuration of this program.
  dwShaderFlags |= D3DCOMPILE_DEBUG;

  // Disable optimizations to further improve shader debugging
  dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  ID3DBlob* pErrorBlob = nullptr;
  hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

  if (FAILED(hr))
  {
    if (pErrorBlob)
    {
      OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
      pErrorBlob->Release();
    }
    return hr;
  }

  if (pErrorBlob)
      pErrorBlob->Release();

  return S_OK;
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

  hr = pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView);
  pBackBuffer->Release();
  if (FAILED(hr))
    return hr;

  pImmediateContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);

  // Setup the viewport
  D3D11_VIEWPORT vp;
  vp.Width = (FLOAT)width;
  vp.Height = (FLOAT)height;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  pImmediateContext->RSSetViewports(1, &vp);

  // Compile the vertex shader
  ID3DBlob* pVSBlob = nullptr;
  hr = CompileShaderFromFile(L"cube_VS.hlsl", "main", "vs_5_0", &pVSBlob);
  if (FAILED(hr))
  {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the vertex shader
  hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pVertexShader);
  if (FAILED(hr))
  {
    pVSBlob->Release();
    return hr;
  }

  // Define the input layout
  D3D11_INPUT_ELEMENT_DESC layout[] =
  {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
  };
  UINT numElements = ARRAYSIZE(layout);

  // Create the input layout
  hr = pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &pVertexLayout);
  pVSBlob->Release();
  if (FAILED(hr))
    return hr;

  // Set the input layout
  pImmediateContext->IASetInputLayout(pVertexLayout);

  // Compile the pixel shader
  ID3DBlob* pPSBlob = nullptr;
  hr = CompileShaderFromFile(L"cube_PS.hlsl", "main", "ps_5_0", &pPSBlob);
  if (FAILED(hr))
  {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the pixel shader
  hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pPixelShader);
  pPSBlob->Release();
  if (FAILED(hr))
    return hr;

  init_time = clock();

  // Create vertex buffer
  SimpleVertex vertices[] = {
       { -1.0f, 1.0f, -1.0f, RGB(0, 0, 255) },
       { 1.0f, 1.0f, -1.0f, RGB(0, 255, 0) },
       { 1.0f, 1.0f, 1.0f, RGB(255, 255, 255) },
       { -1.0f, 1.0f, 1.0f, RGB(255, 0, 0) },
       { -1.0f, -1.0f, -1.0f, RGB(255, 0, 255) },
       { 1.0f, -1.0f, -1.0f, RGB(255, 255, 0) },
       { 1.0f, -1.0f, 1.0f, RGB(0, 255, 255) },
       { -1.0f, -1.0f, 1.0f, RGB(0, 0, 0) }
  };
  USHORT indices[] = {
        3,1,0,
        2,1,3,

        0,5,4,
        1,5,0,

        3,4,7,
        0,4,3,

        1,6,5,
        2,6,1,

        2,7,6,
        3,7,2,

        6,4,5,
        7,4,6,
  };

  D3D11_BUFFER_DESC bd;
  ZeroMemory(&bd, sizeof(bd));
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(vertices);
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;
  bd.MiscFlags = 0;
  bd.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA InitData;
  ZeroMemory(&InitData, sizeof(InitData));
  InitData.pSysMem = &vertices;
  InitData.SysMemPitch = sizeof(vertices);
  InitData.SysMemSlicePitch = 0;

  hr = pd3dDevice->CreateBuffer(&bd, &InitData, &pVertexBuffer);
  if (FAILED(hr))
    return hr;

  // Create index buffer
  D3D11_BUFFER_DESC bd1;
  ZeroMemory(&bd1, sizeof(bd1));
  bd1.Usage = D3D11_USAGE_DEFAULT;
  bd1.ByteWidth = sizeof(indices);
  bd1.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd1.CPUAccessFlags = 0;
  bd1.MiscFlags = 0;
  bd1.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA InitData1;
  ZeroMemory(&InitData1, sizeof(InitData1));
  InitData1.pSysMem = &indices;
  InitData1.SysMemPitch = sizeof(indices);
  InitData1.SysMemSlicePitch = 0;

  hr = pd3dDevice->CreateBuffer(&bd1, &InitData1, &pIndexBuffer);
  if (FAILED(hr))
    return hr;

  // Set constant buffers
  D3D11_BUFFER_DESC descWMB = {};
  descWMB.ByteWidth = sizeof(WorldMatrixBuffer);
  descWMB.Usage = D3D11_USAGE_DEFAULT;
  descWMB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  descWMB.CPUAccessFlags = 0;
  descWMB.MiscFlags = 0;
  descWMB.StructureByteStride = 0;

  WorldMatrixBuffer worldMatrixBuffer;
  worldMatrixBuffer.worldMatrix = DirectX::XMMatrixIdentity();

  D3D11_SUBRESOURCE_DATA data;
  data.pSysMem = &worldMatrixBuffer;
  data.SysMemPitch = sizeof(worldMatrixBuffer);
  data.SysMemSlicePitch = 0;

  hr = pd3dDevice->CreateBuffer(&descWMB, &data, &pWorldMatrixBuffer);
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC descSMB = {};
  descSMB.ByteWidth = sizeof(SceneMatrixBuffer);
  descSMB.Usage = D3D11_USAGE_DYNAMIC;
  descSMB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  descSMB.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  descSMB.MiscFlags = 0;
  descSMB.StructureByteStride = 0;

  hr = pd3dDevice->CreateBuffer(&descSMB, nullptr, &pSceneMatrixBuffer);
  if (FAILED(hr))
    return hr;

  // Set rastrizer state
  D3D11_RASTERIZER_DESC descRastr = {};
  descRastr.AntialiasedLineEnable = false;
  descRastr.FillMode = D3D11_FILL_SOLID;
  descRastr.CullMode = D3D11_CULL_BACK;
  descRastr.DepthBias = 0;
  descRastr.DepthBiasClamp = 0.0f;
  descRastr.FrontCounterClockwise = false;
  descRastr.DepthClipEnable = true;
  descRastr.ScissorEnable = false;
  descRastr.MultisampleEnable = false;
  descRastr.SlopeScaledDepthBias = 0.0f;

  hr = pd3dDevice->CreateRasterizerState(&descRastr, &pRasterizerState);
  if (FAILED(hr))
    return hr;

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

#ifdef _DEBUG
  hr = pImmediateContext->QueryInterface(__uuidof(pAnnotation), reinterpret_cast<void**>(&pAnnotation));
  if (FAILED(hr))
    return hr;
#endif

  return S_OK;
}

void Renderer::HandleInput() {
  // handle camera rotations
  XMFLOAT3 mouseMove = input.IsMouseUsed();
  camera.Move(mouseMove.x, mouseMove.y, mouseMove.z);
}

// Update frame method
bool Renderer::Frame() {
  // update inputs
  input.Frame();
  
  // update camera
  HandleInput();
  camera.Frame();

  // Update world matrix angle
  auto duration = (1.0 * clock() - init_time) / CLOCKS_PER_SEC;

  WorldMatrixBuffer worldMatrixBuffer;
  worldMatrixBuffer.worldMatrix = XMMatrixRotationY((float)duration * angle_velocity);
  pImmediateContext->UpdateSubresource(pWorldMatrixBuffer, 0, nullptr, &worldMatrixBuffer, 0, 0);

  // Get the view matrix
  XMMATRIX mView;
  camera.GetBaseViewMatrix(mView);
  // Get the projection matrix
  XMMATRIX mProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, (FLOAT)input.GetWidth() / (FLOAT)input.GetHeight(), 0.01f, 100.0f);
  D3D11_MAPPED_SUBRESOURCE subresource;
  HRESULT hr = pImmediateContext->Map(pSceneMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
  if (FAILED(hr))
    return FAILED(hr);

  SceneMatrixBuffer& sceneBuffer = *reinterpret_cast<SceneMatrixBuffer*>(subresource.pData);
  sceneBuffer.viewProjectionMatrix = XMMatrixMultiply(mView, mProjection);
  pImmediateContext->Unmap(pSceneMatrixBuffer, 0);

  return SUCCEEDED(hr);
}

HRESULT Renderer::Render() {
  pImmediateContext->ClearState();

  ID3D11RenderTargetView* views[] = { pRenderTargetView };
  pImmediateContext->OMSetRenderTargets(1, views, nullptr);

  // Just clear the backbuffer
  auto duration = (1.0 * clock() - init_time) / CLOCKS_PER_SEC;

  float ClearColor[4] = {
    float(0.5 + 0.5 * sin(0.2 * duration)),
    float(0.5 + 0.5 * sin(0.3 * duration)),
    float(0.5 + 0.5 * sin(0.5 * duration)), 1 }; // RGBA


#ifdef _DEBUG
  pAnnotation->BeginEvent((LPCWSTR)("Clear background"));
#endif
  pImmediateContext->ClearRenderTargetView(pRenderTargetView, ClearColor);
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

  pImmediateContext->RSSetState(pRasterizerState);

  // Render a cube
  // Primitive example of debug markers...
#ifdef _DEBUG
  pAnnotation->BeginEvent((LPCWSTR)"Draw Cube");
  std::string indexBufferName = "Indexes buffer", vertexBufferName = "Vertexes buffer";
  pIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, indexBufferName.size(), indexBufferName.c_str());
  pVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, vertexBufferName.size(), vertexBufferName.c_str());
#endif

  pImmediateContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
  ID3D11Buffer* vertexBuffers[] = { pVertexBuffer };
  UINT strides[] = { 16 };
  UINT offsets[] = { 0 };
  pImmediateContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
  pImmediateContext->IASetInputLayout(pVertexLayout);
  pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  pImmediateContext->VSSetShader(pVertexShader, nullptr, 0);
  pImmediateContext->VSSetConstantBuffers(0, 1, &pWorldMatrixBuffer);
  pImmediateContext->VSSetConstantBuffers(1, 1, &pSceneMatrixBuffer);
  pImmediateContext->PSSetShader(pPixelShader, nullptr, 0);
  pImmediateContext->DrawIndexed(36, 0, 0);

#ifdef _DEBUG
  pAnnotation->EndEvent();
#endif

  return pSwapChain->Present(0, 0);
}

void Renderer::CleanupDevice() {
  camera.Realese();
  input.Realese();

#ifdef _DEBUG
  if (pAnnotation) pAnnotation->Release();
#endif

  if (pRasterizerState) pRasterizerState->Release();
  if (pSceneMatrixBuffer) pSceneMatrixBuffer->Release();
  if (pWorldMatrixBuffer) pWorldMatrixBuffer->Release();
  
  if (pIndexBuffer) pIndexBuffer->Release();
  if (pVertexBuffer) pVertexBuffer->Release();
  if (pVertexLayout) pVertexLayout->Release();
  if (pVertexShader) pVertexShader->Release();
  if (pPixelShader) pPixelShader->Release();
  if (pRenderTargetView) pRenderTargetView->Release();
  
  if (pSwapChain1) pSwapChain1->Release();
  if (pSwapChain) pSwapChain->Release();
  if (pImmediateContext1) pImmediateContext1->Release();
  if (pImmediateContext) pImmediateContext->Release();
  if (pd3dDevice1) pd3dDevice1->Release();
  
#ifdef _DEBUG
  ID3D11Debug* d3dDebug = nullptr;
  pd3dDevice->QueryInterface(IID_PPV_ARGS(&d3dDebug));

  UINT references = pd3dDevice->Release();
  if (references > 1) {
      d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
  }
  d3dDebug->Release();
#else
  if (pd3dDevice) pd3dDevice->Release();
#endif
}

HRESULT Renderer::ResizeWindow(const HWND& hWnd) {
  if (pSwapChain)
  {
    pImmediateContext->OMSetRenderTargets(0, 0, 0);

    // Release all outstanding references to the swap chain's buffers.
    pRenderTargetView->Release();

    HRESULT hr;
    // Preserve the existing buffer count and format.
    // Automatically choose the width and height to match the client rect for HWNDs.
    hr = pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr))
      return hr;

    // Get buffer and create a render-target-view.
    ID3D11Texture2D* pBuffer;
    hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBuffer);
    if (FAILED(hr) || !pBuffer)
       return hr;
    
    hr = pd3dDevice->CreateRenderTargetView(pBuffer, NULL, &pRenderTargetView);
    pBuffer->Release();
    if (FAILED(hr))
        return hr;

    pImmediateContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);

    // Set up the viewport.
    RECT rc;
    GetClientRect(hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    pImmediateContext->RSSetViewports(1, &vp);

    input.Resize(width, height);
  }
  return S_OK;
}
