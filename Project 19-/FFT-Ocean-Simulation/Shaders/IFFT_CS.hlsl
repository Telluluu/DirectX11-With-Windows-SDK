#include "IFFT.hlsli"

groupshared float4 buffer[2][SIZE];

void ButterflyValues(uint step, uint index, out uint2 indices, out float2 twiddle)
{
    const float twoPi = 6.28318530718;
    uint b = Size >> (step + 1);
    uint w = b * (index / b);
    uint i = (w + index) % Size;
    sincos(-twoPi / Size * w, twiddle.y, twiddle.x);
    if (Inverse)
        twiddle.y = -twiddle.y;
    indices = uint2(i, i + b);
}

float4 DoFft(uint threadIndex, float4 input)
{
    buffer[0][threadIndex] = input;
    GroupMemoryBarrierWithGroupSync();
    bool flag = false;
    
    [unroll(LOG_SIZE)]
    for (uint step = 0; step < LOG_SIZE; step++)
    {
        uint2 inputsIndices;
        float2 twiddle;
        ButterflyValues(step, threadIndex, inputsIndices, twiddle);
        float4 v = buffer[flag][inputsIndices.y];
        //一次执行两个计算
        //ak = a0 + w*a1  
        buffer[!flag][threadIndex] = buffer[flag][inputsIndices.x]
		    + float4(ComplexMultiply(twiddle, v.xy), ComplexMultiply(twiddle, v.zw));
        flag = !flag;
        GroupMemoryBarrierWithGroupSync();
    }
    
    return buffer[flag][threadIndex];
}

[numthreads(SIZE, 1, 1)]
void CS( uint3 DTid : SV_DispatchThreadID )
{
    //X和Z轴方向两次一维IFFT
    uint threadIndex = DTid.x;
    uint2 targetIndex;
    if (Direction)
        targetIndex = DTid.yx;
    else
        targetIndex = DTid.xy;

    g_Target[targetIndex] = DoFft(threadIndex, g_Target[targetIndex]);
}