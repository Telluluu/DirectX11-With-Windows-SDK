#define PI 3.1415926f
#define G 9.8f

//一些常量
cbuffer cbInitSettings : register(b0)
{
    float OceanLength; //海洋宽度
    int N;  //纹理宽度
    float2 g_pad0; //对齐16字节打包
}

//用于控制海面的参数
cbuffer cbUpdateSettings : register(b1)
{
    float A;//菲利普频谱参数，影响波浪高度
    float4 WindVelocityAndSeed;//xyzw，xy为风速，zw为随机数种子
    
    float Lambda; //偏移影响
    float HeightScale; //高度影响
    float BubblesScale; //泡沫强度
    float BubblesThreshold; //泡沫阈值
    float deltaTime; // 累积时间
    
    float2 g_pad1; //对齐16字节打包
}

//用于FFT计算的阶数
cbuffer cbns : register(b2)
{
    int Ns;
    float3 g_pad2;
}

RWTexture2D<float4> GaussianRandomRT : register(u0);//保存高斯随机数
RWTexture2D<float4> HeightSpectrumRT : register(u1); //高度频谱
RWTexture2D<float4> DisplaceXSpectrumRT : register(u2); //X偏移频谱
RWTexture2D<float4> DisplaceZSpectrumRT : register(u3); //Z偏移频谱
RWTexture2D<float4> DisplaceRT : register(u4); //最后生成的偏移纹理
RWTexture2D<float4> InputRT : register(u5); //输入
RWTexture2D<float4> OutputRT : register(u6); //输出
RWTexture2D<float4> NormalRT : register(u7); //法线纹理
RWTexture2D<float4> BubblesRT : register(u8); //泡沫纹理

//函数声明
float DonelanBannerDirectionalSpreading(float2 k);
float PositiveCosineSquaredDirectionalSpreading(float2 k);
float Phillips(float2 k);
float Dispersion(float2 k);
float2 Gaussian(float2 id);
uint WangHash(uint seed);
float Rand();
float2 CcomplexMultiply(float2 c1, float2 c2);

//Donelan-Banner方向拓展
float DonelanBannerDirectionalSpreading(float2 k)
{
    float betaS;
    float omegap = 0.855f * G / length(WindVelocityAndSeed.xy);
    float ratio = Dispersion(k) / omegap;

    if (ratio < 0.95f)
    {
        betaS = 2.61f * pow(ratio, 1.3f);
    }
    if(ratio >= 0.95f && ratio < 1.6f)
    {
        betaS = 2.28f * pow(ratio, -1.3f);
    }
    if(ratio > 1.6f)
    {
        float epsilon = -0.4f + 0.8393f * exp(-0.567f * log(ratio * ratio));
        betaS = pow(10, epsilon);
    }
    float theta = atan2(k.y, k.x) - atan2(WindVelocityAndSeed.y, WindVelocityAndSeed.x);

    return betaS / max(1e-7f, 2.0f * tanh(betaS * PI) * pow(cosh(betaS * theta), 2));
}
//正余弦平方方向拓展
float PositiveCosineSquaredDirectionalSpreading(float2 k)
{
    float theta = atan2(k.y, k.x) - atan2(WindVelocityAndSeed.y, WindVelocityAndSeed.x);
    if (theta > -PI / 2.0f && theta < PI / 2.0f)
    {
        return 2.0f / PI * pow(cos(theta), 2);
    }
    else
    {
        return 0;
    }
}


float Phillips(float2 k)
{
    float kLength = length(k);
    float kLength2 = kLength * kLength;
    float kLength4 = kLength2 * kLength4;
    float L = length(WindVelocityAndSeed.xy) * length(WindVelocityAndSeed.xy) / G;
    float L2 = L * L;
    
    //|k^2 ω^2|在波数较多时拟合度差
    //在Tessendorf的论文中提到可以将|{\vec k^2·\vec \omega}|^2修改为exp(-k^2l^2)
    //其中l<<L
    
    float damping = 0.01f;
    float l = L * damping * damping;
    float l2 = l * l;
    
    return A * exp(-1 / (kLength * L2)) / kLength4 * exp(-kLength2 * l2);
}

//以下三个函数用于生成h0(k)= (1/sqrt(2)) (ζ1+ζ2) sqrt(Ph(k)) 中的ζ1和ζ2
//随机种子
uint WangHash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}
//计算均匀分布随机数[0,1)
float Rand(uint rng)
{
    // Xorshift算法
    rng ^= (rng << 13);
    rng ^= (rng >> 17);
    rng ^= (rng << 5);
    return rng / 4294967296.0f;;
}
//计算高斯随机数
float2 Gaussian(float2 id)
{
    //均匀分布随机数
    uint rngState = WangHash(id.y * N + id.x);
    float x1 = Rand(rngState);
    float x2 = Rand(rngState);

    x1 = max(1e-6f, x1);
    x2 = max(1e-6f, x2);
    //计算两个相互独立的高斯随机数
    float g1 = sqrt(-2.0f * log(x1)) * cos(2.0f * PI * x2);
    float g2 = sqrt(-2.0f * log(x1)) * sin(2.0f * PI * x2);

    return float2(g1, g2);
}

//计算弥散ω(k)=sqrt(gk)
float Dispersion(float2 k)
{
    return sqrt(G * length(k));
}
//复数相乘
float2 ComplexMultiply(float2 c1, float2 c2)
{
    return float2(c1.x * c2.x - c1.y * c2.y,
    c1.x * c2.y + c1.y * c2.x);
}