#include "sceneConstantBuffer.h"

// Independet constant buffers for world and view projection matrixes
struct PBRMaterial {
  float3 albedo;
  float roughness;
  float metalness;
};

cbuffer WorldMatrixBuffer : register (b0)
{
  float4x4 worldMatrix;
  PBRMaterial pbrMaterial;
};

struct VS_INPUT
{
  float3 position : POSITION;
  float3 normal : NORMAL;
};

struct PS_INPUT
{
  float4 position : SV_POSITION;
  float4 worldPos : POSITION;
  float3 normal : NORMAL;
};

PS_INPUT main(VS_INPUT input) {
  PS_INPUT output;

  output.worldPos = mul(worldMatrix, float4(input.position, 1.0f));
  output.position = mul(viewProjectionMatrix, output.worldPos);
  output.normal = mul(worldMatrix, input.normal);
  
  return output;
}
