#include "IFFT.hlsli"

float4 DoPostProcess(float4 input, uint2 id)
{
    if (Scale)
        input /= Size * Size;
    if (Permute)
        input *= 1.0 - 2.0 * ((id.x + id.y) % 2);
    return input;
}

//PostProcess
[numthreads(8, 8, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
    g_Target[id.xy] = DoPostProcess(g_Target[id.xy], id.xy);
}