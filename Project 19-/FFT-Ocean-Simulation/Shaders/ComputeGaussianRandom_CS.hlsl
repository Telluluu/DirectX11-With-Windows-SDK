#include "FFT.hlsli"

//�����˹�����
[numthreads(16, 16, 1)]
//ComputeGaussianRandom
void CS( uint3 DTid : SV_DispatchThreadID )
{
    float2 gaussianRandom = Gaussian(DTid.xy);
    GaussianRandomRT[DTid.xy] = float4(gaussianRandom, 0, 0);
}