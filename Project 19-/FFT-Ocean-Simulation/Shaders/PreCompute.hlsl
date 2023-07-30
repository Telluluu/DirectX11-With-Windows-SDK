#include "FFT.hlsli"

float2 vec_k(uint2 id)
{
    int n = id.x - N / 2;
    int m = id.y - N / 2;
    return float2(2.0f * PI * n / kL, 2.0f * PI * m / kL);
}

[numthreads(16, 16, 1)]
//PreCompute
void CS( uint3 DTid : SV_DispatchThreadID )
{
    float2 k = vec_k(DTid.xy);
    g_h0Data[DTid.xy] = H0(k, DTid.xy);
}
//¼ÆËã³ö³õÊ¼ÆµÆ×H0