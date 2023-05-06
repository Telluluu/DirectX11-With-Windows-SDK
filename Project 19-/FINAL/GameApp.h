#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <random>
#include <WinMin.h>
#include "d3dApp.h"
#include "Effects.h"
#include <CameraController.h>
#include <RenderStates.h>
#include <GameObject.h>
#include <Texture2D.h>
#include <Buffer.h>
#include <Collision.h>
#include <ModelManager.h>
#include <TextureManager.h>
#include "ParticleManager.h"

class GameApp : public D3DApp
{
public:
    GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

    // 摄像机模式
    enum class CameraMode { FirstPerson, ThirdPerson, Free };
private:
    bool InitResource();
    void CreateRandomTrees();
    void CreateGuardrails();

private:

    TextureManager m_TextureManager;
    ModelManager m_ModelManager;

    std::unique_ptr<Depth2D> m_pDepthTexture;                           // 深度缓冲区
    GameObject m_Guardrail;                                             // 护栏
    GameObject m_Block[4];                                              // 空气墙
    GameObject m_StreetLamp;                                            // 路灯
    GameObject m_Car;                                                   // 汽车
    GameObject m_CarWheelFL;                                            // 汽车左前轮(front-left)
    GameObject m_CarWheelFR;                                            // 汽车右前轮(front-right)
    GameObject m_CarWheelR;                                             // 汽车后轮(rear wheels)
    GameObject m_Trees;										            // 树
    GameObject m_Ground;										        // 地面
    GameObject m_Road;                                                  // 公路
    std::unique_ptr<Buffer> m_pInstancedBuffer;                         // 树的实例缓冲区
    GameObject m_Skybox;                                                // 天空盒
    ParticleManager m_Rain;                                             // 雨水粒子系统
    ParticleManager m_Fire;                                             // 火焰粒子系统
    SpotLight spotLight[2];                                             // 车前灯
    BasicEffect m_BasicEffect;								            // 对象渲染特效管理
    SkyboxEffect m_SkyboxEffect;                                        // 天空盒特效
    ParticleEffect m_RainEffect;                                        // 雨水特效
    ParticleEffect m_FireEffect;                                        // 火焰特效

    std::unique_ptr<Texture2D> m_pLitTexture;                           // 中间场景缓冲区

    std::shared_ptr<Camera> m_pCamera;				                    // 摄像机
    FirstPersonCameraController m_CameraController;                     // 摄像机控制器
    CameraMode m_CameraMode;								        	// 摄像机模式
    bool m_EnableNormalMap = true;								        // 开启法线贴图
    DirectionalLight sun;                                               // 太阳光

    //PSConstantBuffer m_PSConstantBuffer;			                    // 用于修改用于PS的GPU常量缓冲区的变量
};


#endif