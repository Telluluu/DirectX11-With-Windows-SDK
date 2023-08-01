#define PI 3.1415926f
#define G 9.8f

RWTexture2D<float4> g_h0Data : register(u0);

cbuffer CBPrecompute : register(b0)
{
    uint N; // sections
    float kL; //wave lambda
    float A;
    float2 WindVelocity;
    float3 pad0;
}

cbuffer CBUpdate : register(b1)
{
    float Time;
    float3 pad1;
}


//��������
float DonelanBannerDirectionalSpreading(float2 k);
float Phillips(float2 k);
float Dispersion(float2 k);
float2 Gaussian(float2 id);
uint WangHash(uint seed);
float Rand();
float2 ComplexMultiply(float2 c1, float2 c2);

//Donelan-Banner������չ
float DonelanBannerDirectionalSpreading(float2 k)
{
    float betaS;
    float omegap = 0.855f * G / length(WindVelocity);
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
    float theta = atan2(k.y, k.x) - atan2(WindVelocity.y, WindVelocity.x);

    return betaS / max(1e-7f, 2.0f * tanh(betaS * PI) * pow(cosh(betaS * theta), 2));
}

float4 H0(float2 k, int2 id)
{
    float2 rand = Gaussian(id);
    //float2 rand = float2(1.0f, 1.0f);
    float2 h0 = rand * sqrt(abs(Phillips(k) * DonelanBannerDirectionalSpreading(k)) / 2.0f);
    float2 h0Conj = rand * sqrt(abs(Phillips(-k) * DonelanBannerDirectionalSpreading(-k)) / 2.0f);
    h0Conj.y *= -1.0f;
    return float4(h0, h0Conj);
}

float Phillips(float2 k)
{
    float kLength = length(k);
    kLength = max(0.001f, kLength);
    float kLength2 = kLength * kLength;
    float kLength4 = kLength2 * kLength2;
    float L = length(WindVelocity) * length(WindVelocity) / G;
    float L2 = L * L;
    
    //|k^2 ��^2|�ڲ����϶�ʱ��϶Ȳ�
    //��Tessendorf���������ᵽ���Խ�|{\vec k^2��\vec \omega}|^2�޸�Ϊexp(-k^2 * l^2)
    //����l<<L
    
    float damping = 1.0f;
    float l = L * damping * damping;
    float l2 = l * l;
    
    return A * exp(-1 / (kLength * L2)) / kLength4 / 100.0f /*exp(-kLength2 * l2)*/;
}

//��������������������h0(k)= (1/sqrt(2)) (��1+��2) sqrt(Ph(k)) �еĦ�1�ͦ�2
//�������
uint WangHash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}
//������ȷֲ������[0,1)
float Rand(uint rngState)
{
    // Xorshift�㷨
    rngState ^= (rngState << 13);
    rngState ^= (rngState >> 17);
    rngState ^= (rngState << 5);
    return rngState / 4294967296.0f;;
}
//�����˹�����
float2 Gaussian(float2 id)
{
    //���ȷֲ������
    uint rngState = WangHash(id.y * N + id.x);
    float x1 = Rand(rngState);
    float x2 = Rand(rngState);

    x1 = max(1e-6f, x1);
    x2 = max(1e-6f, x2);
    //���������໥�����ĸ�˹�����
    float g1 = sqrt(-2.0f * log(x1)) * cos(2.0f * PI * x2);
    float g2 = sqrt(-2.0f * log(x1)) * sin(2.0f * PI * x2);

    return float2(g1, g2);
}

//������ɢ��(k)=sqrt(gk)
float Dispersion(float2 k)
{
    return sqrt(G * length(k));
}


float2 vec_k(uint2 id)
{
    int n = id.x - N / 2;
    int m = id.y - N / 2;
    return float2(2.0f * PI * n / kL, 2.0f * PI * m / kL);
}


//�������
float2 ComplexMultiply(float2 c1, float2 c2)
{
    return float2(c1.x * c2.x - c1.y * c2.y,
    c1.x * c2.y + c1.y * c2.x);
}