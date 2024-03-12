#pragma once

#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <directxmath.h>
#include <string>
#include <vector>

#include "light.h"
#include "D3DInclude.h"

#define MAX_LIGHT_SOURCES 10  // Additional - fix constant in shaders

using namespace DirectX;

class Box {
private:
  struct Material {
    float shine;
  };

public:
  HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight, Material material);

  void Release();

  void Resize(int screenWidth, int screenHeight) {};

  void Render(ID3D11DeviceContext* context);

  bool Frame(ID3D11DeviceContext* context, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix, XMVECTOR& cameraPos, std::vector<Light>& lights);

private:
  struct BoxVertex
  {
    XMFLOAT3 pos;       // positional coords
    XMFLOAT3 normal;    // normal vec
    XMFLOAT3 tangent;   // tangent vec
  };

  struct WorldMatrixBuffer {
    XMMATRIX worldMatrix;
    XMFLOAT4 color;
  };

  HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
  Material boxMaterial;

  // dx11 vars
  ID3D11VertexShader* g_pVertexShader = nullptr;
  ID3D11PixelShader* g_pPixelShader = nullptr;
  ID3D11InputLayout* g_pVertexLayout = nullptr;

  ID3D11Buffer* g_pVertexBuffer = nullptr;
  ID3D11Buffer* g_pIndexBuffer = nullptr;
  ID3D11Buffer* g_pSceneMatrixBuffer = nullptr;
  ID3D11RasterizerState* g_pRasterizerState = nullptr;
  ID3D11Buffer *g_pWorldMatrixBuffer = nullptr;

  struct LightableSceneMatrixBuffer {
    XMMATRIX viewProjectionMatrix;
    XMFLOAT4 cameraPos;
    XMINT4 lightCount;
    XMFLOAT4 lightPos[MAX_LIGHT_SOURCES];
    XMFLOAT4 lightColor[MAX_LIGHT_SOURCES];
    XMFLOAT4 ambientColor;
  };
};
