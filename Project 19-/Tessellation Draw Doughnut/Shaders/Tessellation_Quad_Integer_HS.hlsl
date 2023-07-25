

#include "Tessellation.hlsli"

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("QuadConstantHS")]
[maxtessfactor(64.0f)]
float3 HS(
InputPatch<VertexOut, 2> patch,
uint i : SV_OutputControlPointID,
uint patchId : SV_PrimitiveID) : POSITION
{
   float3 center = patch[0].posL.xyz;
   float3 dir = patch[1].posL.xyz;
   float r = patch[0].posL.w;
   float R = patch[1].posL.w;
   float d = R - r;

   float PI = 3.14159265;
   float C = 2 * PI*(r+d/2);
  
   if (i == 0)
      return dir;//传入方向 从其方向推导此点的位置
   if (i == 1)
      return float3(C/2, d / 2, 0) + center;
   if (i == 2)
      return float3(-C / 2, -d / 2, 0) + center;
   if (i == 3)
      return float3(C / 2, -d / 2, 0) + center;

 
   return float3(0,0,0);
}