#pragma once
#include <DirectXMath.h>

using namespace DirectX;

struct PBRMaterial {
	XMFLOAT3 albedo;
	float roughness;
	float metalness;

	PBRMaterial(XMFLOAT3 albedo = XMFLOAT3(0.5f, 0.5f, 0.5f), float roughness = 0.5f, float metalness = 0.5f)
		: albedo(albedo), roughness(roughness), metalness(metalness) {};
};
