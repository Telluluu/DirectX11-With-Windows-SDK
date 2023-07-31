#define SIZE 256
#define LOG_SIZE 8
#define N LOG_SIZE
#define PI 3.1415926f

static uint Size = SIZE;

RWTexture2D<float4> g_Target : register(u0);

cbuffer Params : register(b0)
{
    uint TargetsCount;
    uint Direction; //bool
    uint Inverse; //bool
    uint Scale; //bool
    uint Permute; //bool
};

//用于FFT计算的阶数
cbuffer cbns : register(b1)
{
    int Ns;
    float3 g_pad3;
};


//复数相乘
float2 ComplexMultiply(float2 c1, float2 c2)
{
    return float2(c1.x * c2.x - c1.y * c2.y,
    c1.x * c2.y + c1.y * c2.x);
}