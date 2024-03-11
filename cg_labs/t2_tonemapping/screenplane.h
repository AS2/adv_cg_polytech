#pragma once
#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <vector>

class ScreenPlane
{
public:

	struct ProcessTextureVertex
	{
		struct
		{
			float x;
			float y;
			float z;
		} pos;

		struct
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		} color;

		struct
		{
			float x;
			float y;
		} texcoord;

	};

	HRESULT Init(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext);

	void Render(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext);

	void Release();
	
	void setVS(
		ID3D11Device* pDevice,
		ID3D11DeviceContext* pContext);

private:
	HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	
	const ProcessTextureVertex m_vertices[4] =
	{
		{-1.f,-1.f,0.f, 0,0,0,1, 0.f, 1.f},
		{ 1.f,-1.f,0.f, 0,0,0,1, 1.f, 1.f},
		{-1.f, 1.f,0.f, 0,0,0,1, 0.f, 0.f},
		{ 1.f, 1.f,0.f, 0,0,0,1, 1.f, 0.f},
	};

	const unsigned short m_indices[6] = { 0,3,1,3,0,2 };

	ID3D11Buffer* pVertexBuffer;
	ID3D11Buffer* pIndexBuffer;
	ID3D11VertexShader* pVertexShader;
	
	static constexpr wchar_t const* const m_vsPath = L"CopyVertexShader.cso";
	ID3D11InputLayout* pProcessTextureLayout;
};
