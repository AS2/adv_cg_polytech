#pragma once
#include <DirectXMath.h>

using namespace DirectX;

enum class PBRMode : int
{
	allPBR = 0,
	normal = 1,
	geom = 2,
	fresnel = 3,
};

struct PBRMaterial {
	XMFLOAT3 albedo;
	float roughness;
	float metalness;

	PBRMaterial(XMFLOAT3 albedo = XMFLOAT3(0.5f, 0.5f, 0.5f), float roughness = 0.5f, float metalness = 0.5f)
		: albedo(albedo), roughness(roughness), metalness(metalness) {};
};

struct PBRPoorMaterial {
	float roughness;
	float metalness;
	float dielectricF0;

	PBRPoorMaterial(float roughness = 0.5f, float metalness = 0.5f, float dielectricF0 = 0.5f)
		: roughness(roughness), metalness(metalness), dielectricF0(dielectricF0){};
};
