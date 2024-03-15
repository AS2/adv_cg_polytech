#include "lightCalc.h"

// Independet constant buffers for world and view projection matrixes
cbuffer WorldMatrixBuffer : register (b0)
{
  float4x4 worldMatrix;
  float4 color; // w - specular power
};

struct PS_INPUT
{
  float4 position : SV_POSITION;
  float4 worldPos : POSITION;
  float3 normal : NORMAL;
};

float4 main(PS_INPUT input) : SV_Target0{
  // step 1  - count ambient color
  float3 ambient = color.xyz;//ambientColor.xyz;

  // step 2 - calculate normal
  float3 norm = input.normal;

  // step 3 - return final color with lights
  return float4(CalculateColor(ambient, norm, input.worldPos.xyz, color.w, false), 1.0);
}
