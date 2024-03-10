#pragma once

#include <vector>
#include <d3d11_1.h>
#include "WinDef.h"

class RenderTargetTexture
{
public:
	RenderTargetTexture(int height, int width);

	HRESULT initResource(
		ID3D11Device* const& pDevice,
		ID3D11DeviceContext* const& pContext,
		ID3D11Resource* pBackBuffer = nullptr);
	
	void set(
		ID3D11Device* const& pDevice,
		ID3D11DeviceContext* const& pContext) const;
	
	void clear(
		float red, float green, float blue,
		ID3D11Device* const& pDevice,
		ID3D11DeviceContext* const& pContext);
	
	void setScreenSize(int height, int width) {
		this->height = height;
		this->width = width;
	}

	void Release();

	~RenderTargetTexture();

private:
  int height, width;
  
	ID3D11Texture2D* pTexture2D;
	ID3D11RenderTargetView* pRenderTargetView;
	ID3D11ShaderResourceView* pShaderResourceView;

  D3D11_VIEWPORT vp;
};

