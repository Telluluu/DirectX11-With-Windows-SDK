#include "Tessellation.hlsli"

[domain("quad")]
float4 DS(QuadPatchTess patchTess,
   float2 uv : SV_DomainLocation,
   const OutputPatch<HullOut, 4> quad) : SV_POSITION
{
   float3 dir = normalize(quad[0].posL);
  
   float3 p1 = quad[1].posL;
   float3 p2 = quad[2].posL;
   float3 p3 = quad[3].posL;
   float3 p0 = float3(p2.x, p1.y, p1.z);
   // 双线性插值
   float3 v1 = lerp(p0, p1, uv.x);
   float3 v2 = lerp(p2, p3, uv.x);
   float3 p = lerp(v1, v2, uv.y);
  
   //中心
   float3 center = (p0 + p1 + p2 + p3) * 0.25f;
   //移动到原点
   p -= center;
  
   float d = length(p0 - p2);
  
   float PI = 3.14159265;
   float C = length(p1 - p0);
   float R = C / (2 * PI);
  
   float theta1 = p.y * 2 * PI / d;
   float rho1 = d / 2;
   p = float3(p.x, rho1 * sin(theta1), rho1 * cos(theta1));
  
   float theta2 = p.x / R;
   float rho2 = R + p.z;
   p = float3(rho2 * cos(theta2), p.y, rho2 * sin(theta2));

  
   ////旋转
   //float phi = acos(dir.y);
   //float theta = acos(dir.z);
  
   //float3x3 rot2 = float3x3(
  //   float3(cos(theta), 0, -sin(theta)),
  //   float3(0, 1, 0),
  //   float3(sin(theta), 0, cos(theta))
   //);
  
   //float3x3 rot1 = float3x3(
  //   float3(1, 0, 0),
  //   float3(0, cos(phi), sin(phi)),
  //   float3(0, -sin(phi), cos(phi))
   //);
  
   //float3x3 rot = mul(rot1, rot2);
  
   //p = mul(p, rot);

   //位移
   p+=center;
  
   float4 posH = mul(float4(p, 1.0f), g_WorldViewProj);
  
   return posH;
}
