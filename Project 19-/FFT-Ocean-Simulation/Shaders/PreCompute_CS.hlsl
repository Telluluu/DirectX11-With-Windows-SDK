#include "Phillips.hlsli"

//�����������Ƶ��H0


[numthreads(16, 16, 1)]
//PreCompute
void CS( uint3 DTid : SV_DispatchThreadID )
{
    float2 k = vec_k(DTid.xy);
    g_h0Data[DTid.xy] = H0(k, DTid.xy);
}
//�������ʼƵ��H0