#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <random>
#include <WinMin.h>
#include "d3dApp.h"
#include "Effects.h"
#include "Waves.h"
#include <CameraController.h>
#include <RenderStates.h>
#include <GameObject.h>
#include <Texture2D.h>
#include <Buffer.h>
#include <Collision.h>
#include <ModelManager.h>
#include <TextureManager.h>

//只需要传递一次的cb
struct 
{
    float m_SpatialStep = 0.0f;			// 海洋宽度
    int fftSize;   //fft纹理大小
    DirectX::XMFLOAT2 g_pad0;
} m_CBInitSettings = {};
 

//更新时维护的参数结构体
struct 
{
    float A = 10;            //海浪高度因子
    DirectX::XMFLOAT4 WindVelocityAndSeed = { 0.1f, 0.2f, 0.0, 0.0 };//xyzw,xy为风速, zw为两个随机种子

    float Lambda = -1.0;  //控制偏移大小
    float HeightScale = 1.0;  //高度影响
    float BubblesScale = 1.0;  //泡沫强度
    float BubblesThreshold = 1.0;  //泡沫阈值
    float deltaTime = 0.0f;		// 累积时间

    DirectX::XMFLOAT2 g_pad1;//对齐16字节打包
} m_CBUpdataSettings = {};// 对应Waves.hlsli的常量缓冲区


//专门传递Ns的结构体（用于FFT计算时的阶数）
struct 
{
    int ns;
    DirectX::XMFLOAT3 g_pad2;
}m_CBns = {};

class GameApp : public D3DApp
{
public:

    GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

private:
    bool InitResource();

private:
    
    TextureManager m_TextureManager;
    ModelManager m_ModelManager;

    std::mt19937 m_RandEngine;									// 随机数生成器
    std::uniform_int_distribution<uint32_t> m_RowRange;			// 行索引范围
    std::uniform_int_distribution<uint32_t> m_ColRange;			// 列索引范围
    std::uniform_real_distribution<float> m_MagnitudeRange;		// 振幅范围

    BasicEffect m_BasicEffect;									// 对象渲染特效管理

    GameObject m_Land;											// 地面对象
    GameObject m_WireFence;										// 篱笆盒
    CpuWaves m_CpuWaves;                                        // CPU水波
    GpuWaves m_GpuWaves;                                        // GPU水波

    std::unique_ptr<Depth2D> m_pDepthTexture;                   // 深度纹理
    std::unique_ptr<Texture2D> m_pLitTexture;                   // 场景绘制的缓冲区

    float m_BaseTime = 0.0f;									// 控制水波生成的基准时间
    int m_WavesMode = 1;                                        // 波浪绘制模式，0-GPU，1-CPU
    bool m_EnabledFog = true;									// 开启雾效

    std::shared_ptr<ThirdPersonCamera> m_pCamera;				// 摄像机
};


#endif