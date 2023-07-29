#define PI 3.1415926f
#define G 9.8f

//һЩ����
cbuffer cbInitSettings : register(b0)
{
    float OceanLength; //������
    int N;  //������
    float2 g_pad_0; //����16�ֽڴ��
}

//���ڿ��ƺ���Ĳ���
cbuffer cbUpdateSettings : register(b1)
{
    float A;//������Ƶ�ײ�����Ӱ�첨�˸߶�
    float4 WindVelocityAndSeed;//xyzw��xyΪ���٣�zwΪ���������
}

RWTexture2D<float4> GaussianRandomRT : register(u0);//�����˹�����

//��������
float DonelanBannerDirectionalSpreading(float2 k);
float PositiveCosineSquaredDirectionalSpreading(float2 k);
float phillips(float2 k);
float dispersion(float2 k);
float2 gaussian(float2 id);
uint wangHash(uint seed);
float rand();
float2 complexMultiply(float2 c1, float2 c2);

//Donelan-Banner������չ
float DonelanBannerDirectionalSpreading(float2 k)
{
    float betaS;
    float omegap = 0.855f * G / length(WindVelocityAndSeed.xy);
    float ratio = dispersion(k) / omegap;

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



float phillips(float2 k)
{
    float kLength = length(k);
    float kLength2 = kLength * kLength;
    float kLength4 = kLength2 * kLength4;
    float L = length(WindVelocityAndSeed.xy) * length(WindVelocityAndSeed.xy) / G;
    float L2 = L * L;
    
    //|k^2 ��^2|�ڲ����϶�ʱ��϶Ȳ�
    //��Tessendorf���������ᵽ���Խ�|{\vec k^2��\vec \omega}|^2�޸�Ϊexp(-k^2l^2)
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
float dispersion(float2 k)
{
    return sqrt(G * length(k));
}
//�������
float2 complexMultiply(float2 c1, float2 c2)
{
    return float2(c1.x * c2.x - c1.y * c2.y,
    c1.x * c2.y + c1.y * c2.x);
}