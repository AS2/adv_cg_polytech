#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

class IBLMapsGenerator {
public:
	HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context);

	HRESULT GenerateMaps(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11ShaderResourceView* irrSRV, ID3D11ShaderResourceView* prefSRV);

	ID3D11ShaderResourceView* GetIRRMapSRV() { return g_pIRRMapSRV; };

	void Release();

private:
	HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

	HRESULT GenerateIrranienceMap(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11ShaderResourceView* irrSRV);
	HRESULT GeneratePrefilteredMap(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11ShaderResourceView* prefSRV);

	void SetViewPort(ID3D11DeviceContext* context, UINT width, UINT hight);

	ID3D11PixelShader* g_pIrrCMPixelShader = nullptr;
	ID3D11PixelShader* g_pPrefilPixelShader = nullptr;
	ID3D11VertexShader* g_pVertexShader = nullptr;
	ID3D11SamplerState* g_pSamplerState = nullptr;

	ID3D11Texture2D* g_pIRRTexture = nullptr;
	ID3D11RenderTargetView* g_pIRRTextureRTV = nullptr;

	ID3D11Texture2D* g_pIRRMap = nullptr;
	ID3D11ShaderResourceView* g_pIRRMapSRV = nullptr;

	ID3D11Texture2D* g_pPrefMap = nullptr;
	ID3D11ShaderResourceView* g_pPrefMapSRV = nullptr;

	struct ConstantBuffer
	{
		XMFLOAT4X4 projectionMatrix;
		XMFLOAT4X4 viewProjectionMatrix;
		float roughness;
	};
	ID3D11Buffer* g_pConstantBuffer = nullptr;

	XMMATRIX mProjection;
	XMMATRIX mViews[6];
	XMMATRIX g_mMatrises[6];

	UINT g_irradienceTextureSize;
	UINT g_prefTextureSize;
	UINT g_prefMipMapLevels;
};
