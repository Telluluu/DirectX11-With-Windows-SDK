RWTexture2D<float4> g_HeightSpectrumRT : register(u0);
RWTexture2D<float4> g_DisplaceXSpectrumRT : register(u1);
RWTexture2D<float4> g_DisplaceZSpectrumRT : register(u2);
RWTexture2D<float4> g_DisplaceRT : register(u3);

cbuffer CBGerstner : register(b0)
{
    uint N;
    float HeightScale;
    float Lambda;
    float pad;
}

[numthreads(16, 16, 1)]
//��XYZƫ�ƺϲ�������Gerstner��
void CS(uint3 id : SV_DispatchThreadID)
{
    float y = (g_HeightSpectrumRT[id.xy].x) / (N * N) * HeightScale; //�߶�
    float x = (g_DisplaceXSpectrumRT[id.xy].x) / (N * N) * Lambda; //x��ƫ��
    float z = (g_DisplaceZSpectrumRT[id.xy].x) / (N * N) * Lambda; //z��ƫ��

    g_DisplaceRT[id.xy] = float4(x, y, z, 0);
}