#include "Phillips.hlsli"

//根据菲利普频谱得到高度频谱、X轴Z轴偏移频谱
RWTexture2D<float4> g_Height : register(u1);
RWTexture2D<float4> g_DisplaceXZ : register(u2);
//RWTexture2D<float4> g_DisplaceZ : register(u3);
RWTexture2D<float4> g_Grad : register(u3);



void UpdateH(float2 k, uint2 id)
{
    float omegat = Dispersion(k) * Time;
    float c = cos(omegat);
    float s = sin(omegat);
    float4 h0;
    uint2 nm = id;
    if (id.x < N / 2)
    {
        nm.x = id.x + N / 2;
    }
    else
    {
        nm.x = id.x - N / 2;
    }
    if(id.y < N/2)
    {
        nm.y = id.y + N / 2;
    }
    else
    {
        nm.y = id.y - N / 2;
    }
    h0 = g_h0Data[nm];
    float2 h1 = ComplexMultiply(h0.xy, float2(c, s)); //h0
    float2 h2 = ComplexMultiply(h0.zw, float2(c, -s)); //conj h0

    float2 HTilde = h1 + h2;

    g_Height[id] = float4(HTilde, 0, 0);

}

void UpdateDxzAndGrad(float2 k, uint2 id)
{
    k /= max(0.001f, length(k));
    float2 HTilde = g_Height[id].xy;

    //由高度频谱求出x/z轴偏移频谱
    float2 kxHTilde = ComplexMultiply(float2(0, -k.x), HTilde);
    float2 kzHTilde = ComplexMultiply(float2(0, -k.y), HTilde);
    
    float omegat = dot(k, id);
    float2 w = (cos(omegat), sin(omegat));
    float2 Gradx = ComplexMultiply(kxHTilde, w);
    float2 Gradz = ComplexMultiply(kzHTilde, w);

    //g_DisplaceX[id] = float4(kxHTilde, 0, 0);
    //g_DisplaceZ[id] = float4(kzHTilde, 0, 0);
    g_DisplaceXZ[id] = float4(kxHTilde, kzHTilde);
    g_Grad[id] = float4(Gradx, Gradz);

}


[numthreads(16, 16, 1)]
void CS(uint3 DTid : SV_DispatchThreadID)
{
    float2 k = vec_k(DTid.xy);
    
    UpdateH(k, DTid.xy);
    AllMemoryBarrierWithGroupSync();
    UpdateDxzAndGrad(k, DTid.xy);

}