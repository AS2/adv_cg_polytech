#include "HDRCubeMapGenerator.h"
#include "D3DInclude.h"
#include "renderer.h"

HRESULT HDRCubeMapGenerator::CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
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
  D3DInclude includeObj;

  ID3DBlob* pErrorBlob = nullptr;
  hr = D3DCompileFromFile(szFileName, nullptr, &includeObj, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

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

HRESULT HDRCubeMapGenerator::Init(ID3D11Device* device, ID3D11DeviceContext* context) {
  // Init constants
  g_hdrTextureSize = 512;
  
  g_mMatrises[0] = XMMatrixRotationY(XM_PIDIV2);  // +X
  g_mMatrises[1] = XMMatrixRotationY(-XM_PIDIV2); // -X

  g_mMatrises[2] = XMMatrixRotationX(-XM_PIDIV2); // +Y
  g_mMatrises[3] = XMMatrixRotationX(XM_PIDIV2);  // -Y

  g_mMatrises[4] = XMMatrixIdentity();            // +Z
  g_mMatrises[5] = XMMatrixRotationY(XM_PI);      // -Z

  mViews[0] = DirectX::XMMatrixLookToLH(
    { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }
  );	// +X
  mViews[1] = DirectX::XMMatrixLookToLH(
    { 0.0f, 0.0f, 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }
  );	// -X
  mViews[2] = DirectX::XMMatrixLookToLH(
    { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }
  );	// +Y
  mViews[3] = DirectX::XMMatrixLookToLH(
    { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }
  );	// -Y
  mViews[4] = DirectX::XMMatrixLookToLH(
    { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }
  );	// +Z
  mViews[5] = DirectX::XMMatrixLookToLH(
    { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }
  );	// -Z

  const float nearp = 0.5f;
  const float farp = 1.5f;
  const float fov = XM_PIDIV2;
  const float width = nearp / tanf(fov / 2.0f);
  const float height = width;
  mProjection = DirectX::XMMatrixPerspectiveLH(2 * width, 2 * height, nearp, farp);

  // Compile shaders
  ID3D10Blob* vertexShaderBuffer = nullptr;
  ID3D10Blob* pixelShaderBuffer = nullptr;
  int flags = 0;
#ifdef _DEBUG
  flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  HRESULT hr = D3DCompileFromFile(L"HDRToCubeMap_VS.hlsl", NULL, NULL, "main", "vs_5_0", flags, 0, &vertexShaderBuffer, NULL);
  if (FAILED(hr))
    return hr;

  hr = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &g_pVertexShader);
  if (FAILED(hr))
    return hr;

  hr = D3DCompileFromFile(L"HDRToCubeMap_PS.hlsl", NULL, NULL, "main", "ps_5_0", flags, 0, &pixelShaderBuffer, NULL);
  if (FAILED(hr))
    return hr;

  hr = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &g_pPixelShader);
  if (FAILED(hr))
    return hr;

  // Set constant buffers
  D3D11_BUFFER_DESC descCB = {0};
  descCB.Usage = D3D11_USAGE_DEFAULT;
  descCB.ByteWidth = sizeof(ConstantBuffer);
  descCB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  descCB.CPUAccessFlags = 0;
  descCB.MiscFlags = 0;
  descCB.StructureByteStride = 0;

  ConstantBuffer cb;
  memset(&cb, 0, sizeof(cb));

  D3D11_SUBRESOURCE_DATA data;
  data.pSysMem = &cb;
  hr = device->CreateBuffer(&descCB, &data, &g_pConstantBuffer);
  if (FAILED(hr))
    return hr;

  // Init sampler
  D3D11_SAMPLER_DESC descSmplr = {};

  descSmplr.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  descSmplr.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  descSmplr.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  descSmplr.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  descSmplr.MinLOD = 0;
  descSmplr.MaxLOD = D3D11_FLOAT32_MAX;
  descSmplr.MipLODBias = 0.0f;
  
  hr = device->CreateSamplerState(&descSmplr, &g_pSamplerState);
  if (FAILED(hr))
    return hr;

  // cleate render target texture
  D3D11_TEXTURE2D_DESC hdrtd = {};
  hdrtd.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  hdrtd.Width = g_hdrTextureSize;
  hdrtd.Height = g_hdrTextureSize;
  hdrtd.BindFlags = D3D11_BIND_RENDER_TARGET;
  hdrtd.Usage = D3D11_USAGE_DEFAULT;
  hdrtd.CPUAccessFlags = 0;
  hdrtd.MiscFlags = 0;
  hdrtd.MipLevels = 1;
  hdrtd.ArraySize = 1;
  hdrtd.SampleDesc.Count = 1;
  hdrtd.SampleDesc.Quality = 0;

  hr = device->CreateTexture2D(&hdrtd, nullptr, &g_pHDRTexture);
  if (FAILED(hr))
    return hr;

  hr = device->CreateRenderTargetView(g_pHDRTexture, nullptr, &g_pHDRTextureRTV);
  if (FAILED(hr))
    return hr;

  // Create cube map texture
  D3D11_TEXTURE2D_DESC desc = {};
  desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  desc.Width = g_hdrTextureSize;
  desc.Height = g_hdrTextureSize;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
  desc.MipLevels = 1;
  desc.ArraySize = 6;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;

  hr = device->CreateTexture2D(&desc, nullptr, &g_pCubeMapTexture);
  return hr;
}

void HDRCubeMapGenerator::SetViewPort(ID3D11DeviceContext* context, UINT width, UINT hight)
{
  D3D11_VIEWPORT viewport = {};
  viewport.TopLeftX = 0.0f;
  viewport.TopLeftY = 0.0f;
  viewport.Width = (float)width;
  viewport.Height = (float)hight;
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;

  D3D11_RECT rect = {};
  rect.left = 0;
  rect.top = 0;
  rect.right = width;
  rect.bottom = hight;

  context->RSSetViewports(1, &viewport);
  context->RSSetScissorRects(1, &rect);
}

HRESULT HDRCubeMapGenerator::GenerateCubeMap(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11ShaderResourceView* txtSRV) {
  context->ClearState();
  context->OMSetRenderTargets(1, &g_pHDRTextureRTV, nullptr);
  Renderer::GetInstance().EnableDepth(false);
  
  // set view port & scissors rect
  SetViewPort(context, g_hdrTextureSize, g_hdrTextureSize);

  context->IASetInputLayout(nullptr);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  context->VSSetShader(g_pVertexShader, nullptr, 0);
  context->PSSetShader(g_pPixelShader, nullptr, 0);
  context->PSSetShaderResources(0, 1, &txtSRV);
  context->PSSetSamplers(0, 1, &g_pSamplerState);

  float clearColor[4] = { 0.9f, 0.3f, 0.1f, 1.0f };
  ConstantBuffer cb = {};

  for (UINT i = 0; i < 6; ++i)
  {
    context->ClearRenderTargetView(g_pHDRTextureRTV, clearColor);
    XMStoreFloat4x4(&cb.projectionMatrix, XMMatrixTranspose(g_mMatrises[i]));
    XMStoreFloat4x4(&cb.viewProjectionMatrix, XMMatrixTranspose(mViews[i] * mProjection));

    context->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    context->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    context->Draw(4, 0);
    context->CopySubresourceRegion(g_pCubeMapTexture, i, 0, 0, 0, g_pHDRTexture, 0, nullptr);
  }

  // Create subresource
  HRESULT hr = device->CreateShaderResourceView(g_pCubeMapTexture, nullptr, &g_pCMSRV);
  Renderer::GetInstance().EnableDepth(true);
  return hr;
}

void HDRCubeMapGenerator::Release() {
  if (g_pCMSRV) g_pCMSRV->Release();
  if (g_pCubeMapTexture) g_pCubeMapTexture->Release();
  if (g_pConstantBuffer) g_pConstantBuffer->Release(); 
  if (g_pPixelShader) g_pPixelShader->Release();
  if (g_pVertexShader) g_pVertexShader->Release();
  if (g_pSamplerState) g_pSamplerState->Release();
  if (g_pHDRTextureRTV) g_pHDRTextureRTV->Release();
  if (g_pHDRTexture) g_pHDRTexture->Release();
}