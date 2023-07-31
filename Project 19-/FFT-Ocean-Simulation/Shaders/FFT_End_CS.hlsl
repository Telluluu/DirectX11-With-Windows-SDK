#include "IFFT.hlsli"

[numthreads(16, 16, 1)]
//FFT_End
void CS( uint3 id : SV_DispatchThreadID )
{
    int2 idxs = id.xy;
    idxs.x = floor(id.x / (Ns * 2.0f)) * Ns + id.x % Ns;
    float angle = 2.0f * PI * (id.x / (Ns * 2.0f));
    float2 w = float2(cos(angle), sin(angle));

    /*********不同点***********/
    w *= -1;
    /***************************/

    float2 x0 = g_Target[idxs].xy;
    float2 x1 = g_Target[int2(idxs.x + N * 0.5f, idxs.y)].xy;

    float2 output = x0 + ComplexMultiply(w, x1);
    // + float2(w.x * x1.x - w.y * x1.y, w.x * x1.y + w.y * x1.x);
    /*********不同点***********/
    int x = id.x - N * 0.5f;
    output *= ((x + 1) % 2.0f) * 1 + (x % 2.0f) * (-1);
    /***************************/
    g_Target[id.xy] = float4(output, 0, 0);
}