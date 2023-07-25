
#include "Tessellation.hlsli"
//float3 VS(float3 posL : POSITION) : POSITION
//{
//    return posL;
//}

VertexOut VS(float4 pos : POSITION)
{
    VertexOut vOut;
    vOut.posL = pos;
    return vOut;
}