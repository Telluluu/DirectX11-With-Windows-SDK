#define SIZE 256
#define LOG_SIZE 8


static uint Size = SIZE;

RWTexture2D<float4> g_Target : register(u0);

cbuffer Params
{
    uint TargetsCount;
    uint Direction; //bool
    uint Inverse; //bool
    uint Scale; //bool
    uint Permute; //bool
};

//¸´ÊýÏà³Ë
float2 ComplexMultiply(float2 c1, float2 c2)
{
    return float2(c1.x * c2.x - c1.y * c2.y,
    c1.x * c2.y + c1.y * c2.x);
}