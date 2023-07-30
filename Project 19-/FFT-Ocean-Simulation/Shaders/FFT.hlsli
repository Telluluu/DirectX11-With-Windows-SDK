#define PI 3.1415926f
#define G 9.8f

//һЩ����
cbuffer cbInitSettings : register(b0)
{
    float OceanLength; //������
    int N;  //������
    float2 g_pad0; //����16�ֽڴ��
}

//���ڿ��ƺ���Ĳ���
cbuffer cbUpdateSettings : register(b1)
{
    float A;//������Ƶ�ײ�����Ӱ�첨�˸߶�
    float2 WindVelocity;//����
    
    float Lambda; //ƫ��Ӱ��
    float HeightScale; //�߶�Ӱ��
    float BubblesScale; //��ĭǿ��
    float BubblesThreshold; //��ĭ��ֵ
    float deltaTime; // �ۻ�ʱ��
}

//����FFT����Ľ���
cbuffer cbns : register(b2)
{
    int Ns;
    float3 g_pad1;
}

RWTexture2D<float4> g_h0Data : register(u0); //�߶�Ƶ��
RWTexture2D<float4> HeightSpectrumRT : register(u1); //�߶�Ƶ��
RWTexture2D<float4> DisplaceXZSpectrumRT : register(u2); //XZƫ��Ƶ��
RWTexture2D<float4> DisplaceRT : register(u3); //������ɵ�ƫ������
RWTexture2D<float4> InputRT : register(u4); //����
RWTexture2D<float4> OutputRT : register(u5); //���
RWTexture2D<float4> NormalRT : register(u6); //��������
RWTexture2D<float4> BubblesRT : register(u7); //��ĭ����
RWTexture2D<float4> Grad : register(u8); // �ݶȣ����ڼ��㷨��

//��������
float DonelanBannerDirectionalSpreading(float2 k);
float PositiveCosineSquaredDirectionalSpreading(float2 k);
float Phillips(float2 k);
float Dispersion(float2 k);
float2 Gaussian(float2 id);
uint WangHash(uint seed);
float Rand();
float2 CcomplexMultiply(float2 c1, float2 c2);

//Donelan-Banner������չ
float DonelanBannerDirectionalSpreading(float2 k)
{
    float betaS;
    float omegap = 0.855f * G / length(WindVelocity.xy);
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
//������ƽ��������չ
float PositiveCosineSquaredDirectionalSpreading(float2 k)
{
    float theta = atan2(k.y, k.x) - atan2(WindVelocity.y, WindVelocity.x);
    if (theta > -PI / 2.0f && theta < PI / 2.0f)
    {
        return 2.0f / PI * pow(cos(theta), 2);
    }
    else
    {
        return 0;
    }
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
    kLength = max(0.01f, kLength);
    float kLength2 = kLength * kLength;
    float kLength4 = kLength2 * kLength4;
    float L = length(WindVelocity) * length(WindVelocity) / G;
    float L2 = L * L;
    
    //|k^2 ��^2|�ڲ����϶�ʱ��϶Ȳ�
    //��Tessendorf���������ᵽ���Խ�|{\vec k^2��\vec \omega}|^2�޸�Ϊexp(-k^2 * l^2)
    //����l<<L
    
    float damping = 0.01f;
    float l = L * damping * damping;
    float l2 = l * l;
    
    return A * exp(-1 / (kLength * L2)) / kLength4 * exp(-kLength2 * l2);
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
float Rand(uint rng)
{
    // Xorshift�㷨
    rng ^= (rng << 13);
    rng ^= (rng >> 17);
    rng ^= (rng << 5);
    return rng / 4294967296.0f;;
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
//�������
float2 ComplexMultiply(float2 c1, float2 c2)
{
    return float2(c1.x * c2.x - c1.y * c2.y,
    c1.x * c2.y + c1.y * c2.x);
}