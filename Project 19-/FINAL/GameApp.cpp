#include "GameApp.h"
#include <XUtil.h>
#include <DXTrace.h>
using namespace DirectX;

#pragma warning(disable: 26812)

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight),
    m_CameraMode(CameraMode::FirstPerson),
    //m_PSConstantBuffer(),
    m_EnableNormalMap(false)
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
    if (!D3DApp::Init())
        return false;

    m_TextureManager.Init(m_pd3dDevice.Get());
    m_ModelManager.Init(m_pd3dDevice.Get());

    // 务必先初始化所有渲染状态，以供下面的特效使用
    RenderStates::InitAll(m_pd3dDevice.Get());

    if (!m_BasicEffect.InitAll(m_pd3dDevice.Get()))
        return false;

    if (!m_SkyboxEffect.InitAll(m_pd3dDevice.Get()))
        return false;

    if (!m_FireEffect.InitAll(m_pd3dDevice.Get(), L"Shaders\\Fire.hlsl"))
        return false;

    if (!m_RainEffect.InitAll(m_pd3dDevice.Get(), L"Shaders\\Rain.hlsl"))
        return false;

    if (!InitResource())
        return false;

    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();

    m_pDepthTexture = std::make_unique<Depth2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight);
    m_pLitTexture = std::make_unique<Texture2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

    m_pDepthTexture->SetDebugObjectName("DepthTexture");
    m_pLitTexture->SetDebugObjectName("LitTexture");

    // 摄像机变更显示
    if (m_pCamera != nullptr)
    {
        m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
        m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
        m_BasicEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
        m_SkyboxEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
        m_FireEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
        m_RainEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
    }
}

void GameApp::UpdateScene(float dt)
{
    //获取摄像机子类
    auto cam1st = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);
    auto cam3rd = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);

    Transform& carTransform = m_Car.GetTransform();
    Transform& carWheelFL = m_CarWheelFL.GetTransform();
    Transform& carWheelFR = m_CarWheelFR.GetTransform();
    Transform& carWheelR = m_CarWheelR.GetTransform();

    //获取护栏碰撞盒
    BoundingOrientedBox blockBox[4];
    for (int temp = 0; temp < 4; temp++)
    {
        blockBox[temp] = m_Block[temp].GetBoundingOrientedBox();
        blockBox[temp].Extents.x = blockBox[temp].Extents.x * 1500.0f;
        blockBox[temp].Extents.z = blockBox[temp].Extents.z * 10.0f;
    }
  
    //获取碰撞盒，用于后续车辆旋转操作（obj中车辆原点不为几何中心）
    BoundingOrientedBox carBox = m_Car.GetBoundingOrientedBox();
    BoundingOrientedBox carWheelFLBox = m_CarWheelFL.GetBoundingOrientedBox();
    BoundingOrientedBox carWheelFRBox = m_CarWheelFR.GetBoundingOrientedBox();
    BoundingOrientedBox carWheelRBox = m_CarWheelR.GetBoundingOrientedBox();
    XMFLOAT3 wheelFrontMid =XMFLOAT3((carWheelFLBox.Center.x + carWheelFRBox.Center.x) / 2, (carWheelFLBox.Center.y + carWheelFRBox.Center.y) / 2, (carWheelFLBox.Center.z + carWheelFRBox.Center.z) / 2);
    //获取小车后上方位置，便于调整视角
    Transform carPos = carTransform;
    carPos.Translate(XMFLOAT3(0, 1, 0), 2.5f);
    carPos.Translate(XMFLOAT3(0, 0, 1), 4.0f);

    //加速度与速度
    float accerelation = 0.0f;
    static float speed = 0.0f;
    speed += accerelation;
    speed = std::clamp(speed, -0.1f, 0.1f);

    //旋转参数
    //车辆旋转
    float theta = 0.0f;
    static float nowTheta = 0.0f;
    //昼夜变化
    static float phi2 = -0.577f;
    static float rotationIntensity = 0.001f;
    phi2 += rotationIntensity;
    XMMATRIX W1 = XMMatrixRotationZ(phi2);
    XMFLOAT3 sundir = XMFLOAT3(-0.577f, -0.577f, phi2);
    sun.direction = sundir;
    //m_PSConstantBuffer.rotationZ = XMMatrixTranspose(W1);


    //是否刚刚进行过漂移
    static bool isDrift=0;

    ImGuiIO& io = ImGui::GetIO();
    // ******************
    if (m_CameraMode == CameraMode::FirstPerson || m_CameraMode == CameraMode::Free)
    {
        // 第一人称摄像机的操作
        float d1 = 0.0f, d2 = 0.0f, d3 = 0.0f;
        if (ImGui::IsKeyDown(ImGuiKey_W))
            d1 += dt;
        if (ImGui::IsKeyDown(ImGuiKey_S))
            d1 -= dt;
        if (ImGui::IsKeyDown(ImGuiKey_A))
            d2 -= dt;
        if (ImGui::IsKeyDown(ImGuiKey_D))
            d2 += dt;

        if (m_CameraMode == CameraMode::FirstPerson)
            cam1st->Walk(d1 * 6.0f);
        else
            cam1st->MoveForward(d1 * 6.0f);
        cam1st->Strafe(d2 * 6.0f);

        if (m_CameraMode == CameraMode::Free)
        {
            if (ImGui::IsKeyDown(ImGuiKey_Space))
                d3 += dt*5;
            if (ImGui::IsKeyDown(ImGuiKey_C))
                d3 -= dt*5;
        }
        if (m_CameraMode == CameraMode::Free)
            cam1st->Translate(XMFLOAT3(0.0f, 1.0f, 0.0f), d3);

        if (m_CameraMode == CameraMode::FirstPerson)
        {
            if (ImGui::IsKeyDown(ImGuiKey_W))
            {
                carTransform.Translate(carTransform.GetForwardAxis(), -0.002f);
                carWheelFL.Translate(carTransform.GetForwardAxis(), -0.002f);
                carWheelFR.Translate(carTransform.GetForwardAxis(), -0.002f);
                carWheelR.Translate(carTransform.GetForwardAxis(), -0.002f);
            }
            if (ImGui::IsKeyDown(ImGuiKey_S))
            {
                carTransform.Translate(carTransform.GetForwardAxis(), 0.002f);
                carWheelFL.Translate(carTransform.GetForwardAxis(), 0.002f);
                carWheelFR.Translate(carTransform.GetForwardAxis(), 0.002f);
                carWheelR.Translate(carTransform.GetForwardAxis(), 0.002f);
            }
            if (ImGui::IsKeyDown(ImGuiKey_A))
            {
                carTransform.Translate(carTransform.GetForwardAxis(), 0.002f);
                carWheelFL.Translate(carTransform.GetForwardAxis(), 0.002f);
                carWheelFR.Translate(carTransform.GetForwardAxis(), 0.002f);
                carWheelR.Translate(carTransform.GetForwardAxis(), 0.002f);
            }
            if (ImGui::IsKeyDown(ImGuiKey_D))
            {
                carTransform.Translate(carTransform.GetForwardAxis(), 0.002f);
                carWheelFL.Translate(carTransform.GetForwardAxis(), 0.002f);
                carWheelFR.Translate(carTransform.GetForwardAxis(), 0.002f);
                carWheelR.Translate(carTransform.GetForwardAxis(), 0.002f);
            }
        }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            cam1st->Pitch(io.MouseDelta.y * 0.01f);
            cam1st->RotateY(io.MouseDelta.x * 0.01f);
        }
    }
    else if (m_CameraMode == CameraMode::ThirdPerson)
    {
        // 第三人称摄像机的操作
        cam3rd->SetTarget(carTransform.GetPosition());
        if (!ImGui::IsAnyItemActive())//当ImGui的UI处于非活跃状态时
        {
            // 绕物体旋转
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
            {
                cam3rd->RotateX(io.MouseDelta.y * 0.01f);
                cam3rd->RotateY(io.MouseDelta.x * 0.01f);
            }
            cam3rd->Approach(-io.MouseWheel * 1.0f);
            
           //汽车运动
            {
                //根据速度控制轮胎旋转
                carWheelFL.RotateAround(carWheelFLBox.Center, carWheelFL.GetRightAxis(), -speed * 10);
                carWheelFR.RotateAround(carWheelFRBox.Center, carWheelFR.GetRightAxis(), -speed * 10);
                carWheelR.RotateAround(carWheelRBox.Center, carWheelR.GetRightAxis(), -speed * 10);

                //A/D键控制汽车转弯
                if (ImGui::IsKeyDown(ImGuiKey_A))
                {
                    theta = 0.001f;
                    nowTheta += theta;
                    // 前轮旋转角度不大于40°
                    if (nowTheta < (6.28f / 9.0))
                    {
                        carWheelFL.RotateAround(carWheelFLBox.Center, carTransform.GetUpAxis(), -theta);
                        carWheelFR.RotateAround(carWheelFRBox.Center, carTransform.GetUpAxis(), -theta);
                    }

                    if (speed > 0 && !ImGui::IsKeyDown(ImGuiKey_F))
                    {
                        carTransform.RotateAxis(carTransform.GetUpAxis(), -theta / 2.0f);
                        carWheelR.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), -theta / 2.0f);
                        carWheelFL.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), -theta / 2.0f);
                        carWheelFR.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), -theta / 2.0f);
                    }
                    else if (speed < 0 && !ImGui::IsKeyDown(ImGuiKey_F))
                    {
                        carTransform.RotateAxis(carTransform.GetUpAxis(), theta / 2.0f);
                        carWheelR.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), theta / 2.0f);
                        carWheelFL.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), theta / 2.0f);
                        carWheelFR.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), theta / 2.0f);
                    }
                    
                    if (ImGui::IsKeyDown(ImGuiKey_F)) 
                    {
                        if (speed > 0)
                        {
                            speed = 0.01f;
                            carTransform.RotateAxis(carTransform.GetUpAxis(), -0.002f);
                            carWheelR.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), -0.002f);
                            carWheelFL.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), -0.002f);
                            carWheelFR.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), -0.002f);
                        }
                        else if (speed < 0)
                        {
                            speed = -0.01f;
                            carTransform.RotateAxis(carTransform.GetUpAxis(), 0.002f);
                            carWheelR.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), 0.002f);
                            carWheelFL.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), 0.002f);
                            carWheelFR.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), 0.002f);
                        }
                        isDrift = 1;
                    }
                }
                if (ImGui::IsKeyDown(ImGuiKey_D))
                {
                    theta = 0.001f;
                    nowTheta += theta;
                    // 前轮旋转角度不大于40°
                    if (nowTheta < (6.28f / 9.0))
                    {
                        carWheelFL.RotateAround(carWheelFLBox.Center, carTransform.GetUpAxis(), theta);
                        carWheelFR.RotateAround(carWheelFRBox.Center, carTransform.GetUpAxis(), theta);
                    }

                    if (speed > 0 && !ImGui::IsKeyDown(ImGuiKey_F))
                    {
                        carTransform.RotateAxis(carTransform.GetUpAxis(), theta / 2.0f);
                        carWheelR.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), theta / 2.0f);
                        carWheelFL.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), theta / 2.0f);
                        carWheelFR.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), theta / 2.0f);
                    }
                    else if (speed < 0 && !ImGui::IsKeyDown(ImGuiKey_F))
                    {
                        carTransform.RotateAxis(carTransform.GetUpAxis(), -theta / 2.0f);
                        carWheelR.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), -theta / 2.0f);
                        carWheelFL.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), -theta / 2.0f);
                        carWheelFR.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), -theta / 2.0f);
                    }
                    if (ImGui::IsKeyDown(ImGuiKey_F))
                    {
                        if (speed > 0)
                        {
                            speed = 0.01f;
                            carTransform.RotateAxis(carTransform.GetUpAxis(), 0.002f);
                            carWheelR.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), 0.002f);
                            carWheelFL.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), 0.002f);
                            carWheelFR.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), 0.002f);
                        }
                        else if (speed < 0)
                        {
                            speed = -0.01f;
                            carTransform.RotateAxis(carTransform.GetUpAxis(), -0.002f);
                            carWheelR.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), -0.002f);
                            carWheelFL.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), -0.002f);
                            carWheelFR.RotateAround(carTransform.GetPosition(), carTransform.GetUpAxis(), -0.002f);
                        }
                        isDrift = 1;
                    }
                }
                //松开A/D键时，前轮旋转复原
                if (ImGui::IsKeyReleased(ImGuiKey_A) || ImGui::IsKeyReleased(ImGuiKey_D))
                {
                    carWheelFL.SetRotation(carTransform.GetRotation());
                    carWheelFR.SetRotation(carTransform.GetRotation());
                    carWheelFL.SetPosition(carTransform.GetPosition());
                    carWheelFR.SetPosition(carTransform.GetPosition());
                    nowTheta = 0.0f;
                }

                //松开刹车时，W/S键控制汽车加速度
                for (int temp = 0; temp < 4; temp++)
                    if (!carBox.Intersects(blockBox[temp]))
                    {
                        if (!ImGui::IsKeyDown(ImGuiKey_Space))
                        {
                            if (ImGui::IsKeyDown(ImGuiKey_W) || (ImGui::IsMouseDown(ImGuiMouseButton_Right) && ImGui::IsMouseDown(ImGuiMouseButton_Left)))
                                if (speed > 0)
                                    accerelation = 0.000002f;
                                else accerelation = 0.000004f;
                            if (ImGui::IsKeyDown(ImGuiKey_S))
                                if (speed < 0)
                                    accerelation = -0.000002f;
                                else accerelation = -0.000004f;
                        }
                        //speed += accerelation;
                        speed = std::clamp(speed, -0.05f, 0.05f);
                    }
                    else
                    {
                        //accerelation = -accerelation;
                        if (speed >= 0.005f || speed <= -0.005f)
                        {
                            if (speed > 0.2f)
                                speed -= 0.1f;
                            if (speed < -0.2f)
                                speed += 0.1f;
                            speed = -speed;
                            accerelation = -speed / std::abs(speed) * accerelation;
                        }
                        else if (speed > 0)
                            speed = -0.005f;
                        else if (speed < 0)
                            speed = 0.005f;
                    }
                if (ImGui::IsKeyReleased(ImGuiKey_F) && isDrift)
                {
                    //漂移可以获得向前的加速度，若倒车时漂移就没有加速度了
                    if (speed > 0)
                        speed = 0.03f;
                    isDrift = 0;
                }

                //空格刹车
                if (ImGui::IsKeyDown(ImGuiKey_Space) && speed != 0.0f)
                {
                    if (speed > 0.00002f)
                        accerelation = -0.00002f;
                    else if (speed < -0.00002f)
                        accerelation = 0.00002f;
                    speed += accerelation;
                    if ((speed >= -0.00002f) && (speed <= 0.00002f))
                        speed = 0.0f;
                }

                speed += accerelation;
                speed = std::clamp(speed, -0.1f, 0.1f);

                //进行玩旋转操作后再平移，防止物体变形
                //汽车车身、前后轮在速度方向上平移
                carTransform.Translate(carTransform.GetForwardAxis(), -speed);
                carWheelFL.Translate(carTransform.GetForwardAxis(), -speed);
                carWheelFR.Translate(carTransform.GetForwardAxis(), -speed);
                carWheelR.Translate(carTransform.GetForwardAxis(), -speed);
            }

            //车前灯
            if (ImGui::IsKeyPressed(ImGuiKey_Q))
            {
                if (spotLight[0].range == 0.0f)
                    spotLight[0].range = 10000.0f;
                else if (spotLight[0].range == 10000.0f)
                    spotLight[0].range = 0.0f;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_E))
            {
                if (spotLight[1].range == 0.0f)
                    spotLight[1].range = 10000.0f;
                else if (spotLight[1].range == 10000.0f)
                    spotLight[1].range = 0.0f;
            }
        }
    }
    //获取车前灯位置
    Transform lightFL, lightFR;
    lightFL = carTransform;
    lightFL.Translate(carTransform.GetForwardAxis(), -2.0f);
    lightFL.Translate(carTransform.GetRightAxis(), 1.0f);
    lightFL.Translate(carTransform.GetUpAxis(), 0.5f);
    lightFR = lightFL;
    lightFR.Translate(carTransform.GetRightAxis(), -2.0f);
    //更新车前灯位置和方向
    spotLight[0].position = lightFL.GetPosition();
    spotLight[1].position = lightFR.GetPosition();
    spotLight[0].direction = XMFLOAT3(-carTransform.GetForwardAxis().x ,carTransform.GetForwardAxis().y, -carTransform.GetForwardAxis().z); 
    spotLight[1].direction = XMFLOAT3(-carTransform.GetForwardAxis().x, carTransform.GetForwardAxis().y, -carTransform.GetForwardAxis().z);
    for (int i = 0; i < 2; ++i)
        m_BasicEffect.SetSpotLight(i, spotLight[i]);
    
    if (ImGui::Begin("Car Simulator"))
    {
        if (m_pCamera == cam1st)
            ImGui::Text("Now is 1st p.p.");
        else if(m_pCamera==cam3rd)
            ImGui::Text("Now is 3rd p.p.");
        ImGui::Text("WSAD to control movement\nSpace to stop\nF to drift\nQ/E to use light\n");
        ImGui::Checkbox("Enable Normalmap", &m_EnableNormalMap);
        if (ImGui::Button("Reset Particle"))
        {
            m_Fire.Reset();
            m_Rain.Reset();
        }
        static int curr_item = 0;
        static const char* modes[] = {
            "First Person",
            "Third Person",
            "Free Camera"
        };
        if (ImGui::Combo("Camera Mode", &curr_item, modes, ARRAYSIZE(modes)))
        {
            if (curr_item == 0 && m_CameraMode != CameraMode::FirstPerson)
            {
                if (!cam1st)
                {
                    cam1st.reset(new FirstPersonCamera);
                    cam1st->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
                    m_pCamera = cam1st;
                    OnResize();
                }
                cam1st->LookAt(carPos.GetPosition(),
                    carTransform.GetPosition(),
                    XMFLOAT3(0.0f, 1.0f, 0.0f));

                m_CameraMode = CameraMode::FirstPerson;
            }
            else if (curr_item == 1 && m_CameraMode != CameraMode::ThirdPerson)
            {
                if (!cam3rd)
                {
                    cam3rd = std::make_shared<ThirdPersonCamera>();
                    cam3rd->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
                    m_pCamera = cam3rd;
                    OnResize();
                }
                cam3rd->SetTarget(carPos.GetPosition());
                cam3rd->SetDistance(8.0f);
                cam3rd->SetDistanceMinMax(2.0f, 20.0f);
                cam3rd->LookAt(carPos.GetPosition(),
                    carTransform.GetPosition(),
                    XMFLOAT3(0.0f, 1.0f, 0.0f));
                m_CameraMode = CameraMode::ThirdPerson;
            }
            else if (curr_item == 2 && m_CameraMode != CameraMode::Free)
            {
                if (!cam1st)
                {
                    cam1st.reset(new FirstPersonCamera);
                    cam1st->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
                    m_pCamera = cam1st;
                    OnResize();
                }
                // 从车辆上方开始
                XMFLOAT3 pos = carTransform.GetPosition();
                XMFLOAT3 to = XMFLOAT3(0.0f, 0.0f, 1.0f);
                XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
                pos.y += 3;
                cam1st->LookTo(pos, to, up);

                m_CameraMode = CameraMode::Free;
            }
        }
    }
    ImGui::End();
    ImGui::Render();


    m_SkyboxEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
    m_BasicEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
    m_BasicEffect.SetEyePos(m_pCamera->GetPosition());
    // ******************
    // 粒子系统
    //
    m_Fire.Update(dt, m_Timer.TotalTime());
    m_Rain.Update(dt, m_Timer.TotalTime());

    m_FireEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
    m_FireEffect.SetEyePos(m_pCamera->GetPosition());

    m_RainEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
    m_RainEffect.SetEyePos(m_pCamera->GetPosition());

    static XMFLOAT3 lastCameraPos = m_pCamera->GetPosition();
    XMFLOAT3 cameraPos = m_pCamera->GetPosition();

    XMVECTOR cameraPosVec = XMLoadFloat3(&cameraPos);
    XMVECTOR lastCameraPosVec = XMLoadFloat3(&lastCameraPos);
    XMFLOAT3 emitPos;
    XMStoreFloat3(&emitPos, cameraPosVec + 3.0f * (cameraPosVec - lastCameraPosVec));
    m_RainEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
    m_RainEffect.SetEyePos(m_pCamera->GetPosition());
    m_Rain.SetEmitPos(emitPos);
    lastCameraPos = m_pCamera->GetPosition();
}

void GameApp::DrawScene()
{
    // 创建后备缓冲区的渲染目标视图
    if (m_FrameCount < m_BackBufferCount)
    {
        ComPtr<ID3D11Texture2D> pBackBuffer;
        m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
        CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
        m_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc, m_pRenderTargetViews[m_FrameCount].ReleaseAndGetAddressOf());
    }

    // ******************
    // 正常绘制场景
    //
    
    float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    //清空深度/模板缓冲区和渲染目标视图
    m_pd3dImmediateContext->ClearRenderTargetView(m_pLitTexture->GetRenderTarget(), black);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthTexture->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    ID3D11RenderTargetView* pRTVs[]{ m_pLitTexture->GetRenderTarget() };
    m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthTexture->GetDepthStencil());
    D3D11_VIEWPORT vp = m_pCamera->GetViewPort();
    m_pd3dImmediateContext->RSSetViewports(1, &vp);



    m_BasicEffect.SetRenderDefault();    
    // ******************
    // 1. 给镜面反射区域写入值1到模板缓冲区
    // 

    // 裁剪掉背面三角形
    // 标记镜面区域的模板值为1
    // 不写入像素颜色
    m_pd3dImmediateContext->RSSetState(nullptr);
    m_pd3dImmediateContext->OMSetDepthStencilState(RenderStates::DSSWriteStencil.Get(), 1);
    m_pd3dImmediateContext->OMSetBlendState(RenderStates::BSNoColorWrite.Get(), nullptr, 0xFFFFFFFF);
    //绘制汽车
    m_Car.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

    // ******************
    // 2. 绘制不透明的反射物体
    //

    // 开启反射绘制


    m_CarWheelFL.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    m_CarWheelFR.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    m_CarWheelR.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);


    ////绘制树木
    //m_BasicEffect.DrawInstanced(m_pd3dImmediateContext.Get(), *m_pInstancedBuffer, m_Trees, 144);
    m_BasicEffect.DrawInstanced(m_pd3dImmediateContext.Get(), *m_pInstancedBuffer, m_Guardrail, 360);

    //绘制地面
    m_Ground.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

    //绘制公路
    if (m_EnableNormalMap)
        m_BasicEffect.SetRenderWithNormalMap();
    else
        m_BasicEffect.SetRenderDefault();
    m_Road.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

    //绘制空气墙
    m_BasicEffect.SetRenderDefault();
    for(int temp=0;temp<4;temp++)
        m_Block[temp].Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

    // ******************
    // 绘制天空盒
    //
    pRTVs[0] = GetBackBufferRTV();
    m_pd3dImmediateContext->RSSetViewports(1, &vp);
    m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, nullptr);
    m_SkyboxEffect.SetRenderDefault();
    m_SkyboxEffect.SetDepthTexture(m_pDepthTexture->GetShaderResource());
    m_SkyboxEffect.SetLitTexture(m_pLitTexture->GetShaderResource());
    m_Skybox.Draw(m_pd3dImmediateContext.Get(), m_SkyboxEffect);
    m_SkyboxEffect.SetDepthTexture(nullptr);
    m_SkyboxEffect.SetLitTexture(nullptr);
    m_SkyboxEffect.Apply(m_pd3dImmediateContext.Get());

    // ******************
    // 粒子系统留在最后绘制便于混合
    
    m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthTexture->GetDepthStencil());
    //m_Fire.Draw(m_pd3dImmediateContext.Get(), m_FireEffect);
    //m_Rain.Draw(m_pd3dImmediateContext.Get(), m_RainEffect);


    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));
}

bool GameApp::InitResource()
{
    // ******************
    // 初始化摄像机
    //
    m_CameraMode = CameraMode::FirstPerson;
    auto camera = std::make_shared<FirstPersonCamera>();
    m_pCamera = camera;
    camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
    camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
    camera->LookTo(XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

    // ******************
    // 初始化特效
    //

    m_BasicEffect.SetViewMatrix(camera->GetViewMatrixXM());
    m_BasicEffect.SetProjMatrix(camera->GetProjMatrixXM());

    m_FireEffect.SetBlendState(RenderStates::BSAlphaWeightedAdditive.Get(), nullptr, 0xFFFFFFFF);
    m_FireEffect.SetDepthStencilState(RenderStates::DSSNoDepthWrite.Get(), 0);
    m_FireEffect.SetViewMatrix(camera->GetViewMatrixXM());
    m_FireEffect.SetProjMatrix(camera->GetProjMatrixXM());

    m_RainEffect.SetDepthStencilState(RenderStates::DSSNoDepthWrite.Get(), 0);
    m_RainEffect.SetViewMatrix(camera->GetViewMatrixXM());
    m_RainEffect.SetProjMatrix(camera->GetProjMatrixXM());

    m_SkyboxEffect.SetViewMatrix(camera->GetViewMatrixXM());
    m_SkyboxEffect.SetProjMatrix(camera->GetProjMatrixXM());



    // ******************
    // 初始化游戏对象
    //
    
    // 初始化汽车
    Model* pModel = m_ModelManager.CreateFromFile("..\\Model\\sportcar.017\\sportcar.obj");
    m_Car.SetModel(pModel);
    pModel->SetDebugObjectName("Car");

    pModel = m_ModelManager.CreateFromFile("..\\Model\\sportcar.017\\WheelFL.obj");
    m_CarWheelFL.SetModel(pModel);
    pModel->SetDebugObjectName("CarWheelFL");

    pModel = m_ModelManager.CreateFromFile("..\\Model\\sportcar.017\\WheelFR.obj");
    m_CarWheelFR.SetModel(pModel);
    pModel->SetDebugObjectName("CarWheelFR");

    pModel = m_ModelManager.CreateFromFile("..\\Model\\sportcar.017\\WheelR.obj");
    m_CarWheelR.SetModel(pModel);
    pModel->SetDebugObjectName("CarWheelR");

    // 获取汽车包围盒
    //XMMATRIX S = XMMatrixScaling(0.015f, 0.015f, 0.015f);
    BoundingBox carBox = m_Car.GetModel()->boundingbox;
    BoundingBox carWheelRBox = m_CarWheelR.GetModel()->boundingbox;
    //carBox.Transform(carBox, S);
    // 让汽车底部(轮子)紧贴地面
    Transform& carTransform = m_Car.GetTransform();
    Transform& carWheelFL = m_CarWheelFL.GetTransform();
    Transform& carWheelFR = m_CarWheelFR.GetTransform();
    Transform& carWheelR = m_CarWheelR.GetTransform();
    //carTransform.SetScale(0.015f, 0.015f, 0.015f);
    carTransform.SetPosition(0.0f, -(carWheelRBox.Center.y - carWheelRBox.Extents.y + 2.0f), 0.0f);
    carWheelFL = carTransform;
    carWheelFR = carTransform;
    carWheelR = carTransform;

    // 创建随机的树
    CreateRandomTrees();
    //创建护栏
    CreateGuardrails();

    // 初始化地面
    pModel = m_ModelManager.CreateFromFile("..\\Model\\ground_35.obj");
    pModel->SetDebugObjectName("Ground");
    m_Ground.SetModel(pModel);


    //初始化公路
    pModel = m_ModelManager.CreateFromGeometry("Road", Geometry::CreatePlane(20.0f,1000.0f));
    pModel->SetDebugObjectName("Road");
    m_TextureManager.AddTexture("Road_8k", m_TextureManager.CreateFromFile("..\\Texture\\Road_8k.jpg"));
    pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\Road_8k.jpg");
    m_TextureManager.AddTexture("Road_8kN", m_TextureManager.CreateFromFile("..\\Texture\\Road_8k_nmap.png"));
    pModel->materials[0].Set<std::string>("$Normal", "..\\Texture\\Road_8k_nmap.png");
    pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
    pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
    pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
    pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
    pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
    m_Road.GetTransform().Translate(XMFLOAT3(0.0f, 1.0f, 0.0f), -1.99f);
    m_Road.SetModel(pModel);



    //初始化路灯
    pModel = m_ModelManager.CreateFromFile("..\\Model\\Street Lamp\\StreetLamp.3ds");
    pModel->SetDebugObjectName("StreetLamp");
    m_StreetLamp.SetModel(pModel);
    Transform& streetLamp = m_StreetLamp.GetTransform();
    streetLamp.RotateAxis(XMFLOAT3(0.0f, 1.0f, 0.0f), 3.14f / 2);

    //初始化空气墙Block
    for (int i = 0; i < 2; i++)
    {
        static float sign = 1.0f;
        pModel = m_ModelManager.CreateFromGeometry("Blocks", Geometry::CreatePlane(20.0f,40.0f,1.0f));
        pModel->SetDebugObjectName("BlockZ");
        float j = 500.0f;
        m_Block[i].SetModel(pModel);
        m_Block[i].GetTransform().RotateAxis(XMFLOAT3(1.0f, 0.0f, 0.0f), (3.1415925f / 2.0f));
        m_Block[i].GetTransform().Translate(XMFLOAT3(0.0f, 0.0f, 1.0f), sign*j);
        m_Block[i].SetVisible(0);
        j = -j;
        sign = -sign;
    }
    for (int i = 2; i < 4; i++)
    {
        static float sign = 1.0f;
        pModel = m_ModelManager.CreateFromGeometry("Block", Geometry::CreatePlane(1000.0f,40.0f,1.0f));
        pModel->SetDebugObjectName("BlockX");
        float j = 10.0f;
        m_Block[i].SetModel(pModel);
        m_Block[i].GetTransform().RotateAxis(XMFLOAT3(0.0f, 0.0f, 1.0f), (sign* 3.1415925f / 2.0f));
        m_Block[i].GetTransform().RotateAxis(XMFLOAT3(1.0f, 0.0f, .0f), (3.1415925f / 2.0f));
        m_Block[i].GetTransform().Translate(XMFLOAT3(1.0f, 0.0f, 0.0f), sign*j);
        m_Block[i].SetVisible(0);

        j = -j;
        sign = -sign;
    }

    // 天空盒
    {
        Model* pModel = m_ModelManager.CreateFromGeometry("Skybox", Geometry::CreateBox());
        pModel->SetDebugObjectName("Skybox");
        m_Skybox.SetModel(pModel);
        m_TextureManager.CreateFromFile("..\\Texture\\grasscube1024.dds", false, true);
        pModel->materials[0].Set<std::string>("$Skybox", "..\\Texture\\grasscube1024.dds");
    }

    // ******************
    // 初始化粒子系统
    //
    m_TextureManager.CreateFromFile("..\\Texture\\flare0.dds", false, true);
    m_TextureManager.CreateFromFile("..\\Texture\\raindrop.dds", false, true);

    // 创建随机数据
    std::mt19937 randEngine;
    randEngine.seed(std::random_device()());
    std::uniform_real_distribution<float> randF(-1.0f, 1.0f);
    std::vector<float> randomValues(4096);
    
    // 生成1D随机纹理
    CD3D11_TEXTURE1D_DESC texDesc(DXGI_FORMAT_R32G32B32A32_FLOAT, 1024, 1, 1);
    D3D11_SUBRESOURCE_DATA initData{ randomValues.data(), 1024 * GetFormatSize(DXGI_FORMAT_R32G32B32A32_FLOAT) };
    ComPtr<ID3D11Texture1D> pRandomTex;
    ComPtr<ID3D11ShaderResourceView> pRandomTexSRV;

    std::generate(randomValues.begin(), randomValues.end(), [&]() { return randF(randEngine); });
    HR(m_pd3dDevice->CreateTexture1D(&texDesc, &initData, pRandomTex.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateShaderResourceView(pRandomTex.Get(), nullptr, pRandomTexSRV.ReleaseAndGetAddressOf()));
    m_TextureManager.AddTexture("FireRandomTex", pRandomTexSRV.Get());

    m_Fire.InitResource(m_pd3dDevice.Get(), 500);
    m_Fire.SetTextureInput(m_TextureManager.GetTexture("..\\Texture\\flare0.dds"));
    m_Fire.SetTextureRandom(m_TextureManager.GetTexture("FireRandomTex"));
    m_Fire.SetEmitPos(XMFLOAT3(0.0f, -1.0f, 0.0f));
    m_Fire.SetEmitDir(XMFLOAT3(0.0f, 1.0f, 0.0f));
    m_Fire.SetAcceleration(XMFLOAT3(0.0f, 7.8f, 0.0f));
    m_Fire.SetEmitInterval(0.005f);
    m_Fire.SetAliveTime(1.0f);
    m_Fire.SetDebugObjectName("Fire");
    
    std::generate(randomValues.begin(), randomValues.end(), [&]() { return randF(randEngine); });
    HR(m_pd3dDevice->CreateTexture1D(&texDesc, &initData, pRandomTex.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateShaderResourceView(pRandomTex.Get(), nullptr, pRandomTexSRV.ReleaseAndGetAddressOf()));
    m_TextureManager.AddTexture("RainRandomTex", pRandomTexSRV.Get());

    m_Rain.InitResource(m_pd3dDevice.Get(), 10000);
    m_Rain.SetTextureInput(m_TextureManager.GetTexture("..\\Texture\\raindrop.dds"));
    m_Rain.SetTextureRandom(m_TextureManager.GetTexture("RainRandomTex"));
    m_Rain.SetEmitDir(XMFLOAT3(0.0f, -1.0f, 0.0f));
    m_Rain.SetAcceleration(XMFLOAT3(-1.0f, -9.8f, 0.0f));
    m_Rain.SetEmitInterval(0.0015f);
    m_Rain.SetAliveTime(3.0f);
    m_Rain.SetDebugObjectName("Rain");

    
    // ******************
    // 初始化光照
    //
    // 方向光(默认)
    DirectionalLight dirLight[4];
    dirLight[0].ambient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
    dirLight[0].diffuse = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
    dirLight[0].specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
    dirLight[0].direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
    dirLight[1] = dirLight[0];
    dirLight[1].direction = XMFLOAT3(0.577f, -0.577f, 0.577f);
    dirLight[2] = dirLight[0];
    dirLight[2].direction = XMFLOAT3(0.577f, -0.577f, -0.577f);
    dirLight[3] = dirLight[0];
    dirLight[3].direction = XMFLOAT3(-0.577f, -0.577f, -0.577f);
    for (int i = 0; i < 4; ++i)
        m_BasicEffect.SetDirLight(i, dirLight[i]);

    //聚光灯(车辆车前灯)
    spotLight[0].ambient = XMFLOAT4(0.75f, 0.75f, 0.75f, 1.0f);
    spotLight[0].diffuse = XMFLOAT4(0.85f, 0.85f, 0.85f, 1.0f);
    spotLight[0].specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    spotLight[0].att = XMFLOAT3(1.0f, 0.0f, 0.0f);
    spotLight[0].spot = 12.0f;
    spotLight[0].range = 0.0f;
    spotLight[1] = spotLight[0];
    for (int i = 0; i < 2; ++i)
        m_BasicEffect.SetSpotLight(i, spotLight[i]);

    return true;
}

void GameApp::CreateGuardrails()
{
    // 初始化护栏
    Model* pModel = m_ModelManager.CreateFromFile("..\\Model\\Guardrail\\Guardrail3.obj");
    pModel->SetDebugObjectName("Guardrail");
    m_Guardrail.SetModel(pModel);
    XMMATRIX S = XMMatrixScaling(0.03f, 0.03f, 0.03f);

    BoundingBox guardrailBox = m_Guardrail.GetModel()->boundingbox;
    
    // 让护栏底部紧贴地面位于y = -2的平面
    guardrailBox.Transform(guardrailBox, S);
    float Ty = -(guardrailBox.Center.y - guardrailBox.Extents.y + 2.0f);
    //生成护栏实例数据
    std::vector<BasicEffect::InstancedData> guardrailData(720);
    //创建实例缓冲区
    m_pInstancedBuffer = std::make_unique<Buffer>(m_pd3dDevice.Get(),
        CD3D11_BUFFER_DESC(sizeof(BasicEffect::InstancedData) * 720, D3D11_BIND_VERTEX_BUFFER,
            D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE));
    m_pInstancedBuffer->SetDebugObjectName("InstancedBuffer");


    Transform transform;
    transform.SetScale(0.03f, 0.03f, 0.03f);
    float pos_x = -9.5f , pos_z=-500.0f;
    for (int i = 0; i < 360; ++i)
    {
        transform.SetPosition(pos_x, Ty, pos_z);
        pos_x = -pos_x;
        pos_z += 3.0f;
        XMStoreFloat4x4(&guardrailData[i].world,
            XMMatrixTranspose(transform.GetLocalToWorldMatrixXM()));
        XMStoreFloat4x4(&guardrailData[i].worldInvTranspose,
            XMMatrixTranspose(XMath::InverseTranspose(transform.GetLocalToWorldMatrixXM())));
    }

    // 上传实例数据
    memcpy_s(m_pInstancedBuffer->MapDiscard(m_pd3dImmediateContext.Get()), m_pInstancedBuffer->GetByteWidth(),
        guardrailData.data(), guardrailData.size() * sizeof(BasicEffect::InstancedData));
    m_pInstancedBuffer->Unmap(m_pd3dImmediateContext.Get());
}

void GameApp::CreateRandomTrees()
{
    // 初始化树
    Model* pModel = m_ModelManager.CreateFromFile("..\\Model\\tree.obj");
    pModel->SetDebugObjectName("Trees");
    m_Trees.SetModel(pModel);
    XMMATRIX S = XMMatrixScaling(0.015f, 0.015f, 0.015f);

    BoundingBox treeBox = m_Trees.GetModel()->boundingbox;

    // 让树木底部紧贴地面位于y = -2的平面
    treeBox.Transform(treeBox, S);
    float Ty = -(treeBox.Center.y - treeBox.Extents.y + 2.0f);
    // 随机生成144颗随机朝向的树
    std::vector<BasicEffect::InstancedData> treeData(144);
    m_pInstancedBuffer = std::make_unique<Buffer>(m_pd3dDevice.Get(),
        CD3D11_BUFFER_DESC(sizeof(BasicEffect::InstancedData) * 144, D3D11_BIND_VERTEX_BUFFER,
            D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE));
    m_pInstancedBuffer->SetDebugObjectName("InstancedBuffer");

    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_real<float> radiusNormDist(0.0f, 30.0f);
    std::uniform_real<float> normDist;
    float theta = 0.0f;
    int pos = 0;
    Transform transform;
    transform.SetScale(0.015f, 0.015f, 0.015f);
    for (int i = 0; i < 16; ++i)
    {
        // 取5-95的半径放置随机的树
        for (int j = 0; j < 3; ++j)
        {
            // 距离越远，树木越多
            for (int k = 0; k < 2 * j + 1; ++k, ++pos)
            {
                float radius = (float)(radiusNormDist(rng) + 30 * j + 5);
                float randomRad = normDist(rng) * XM_2PI / 16;
                transform.SetRotation(0.0f, normDist(rng) * XM_2PI, 0.0f);
                transform.SetPosition(radius * cosf(theta + randomRad), Ty, radius * sinf(theta + randomRad));

                XMStoreFloat4x4(&treeData[pos].world,
                    XMMatrixTranspose(transform.GetLocalToWorldMatrixXM()));
                XMStoreFloat4x4(&treeData[pos].worldInvTranspose,
                    XMMatrixTranspose(XMath::InverseTranspose(transform.GetLocalToWorldMatrixXM())));
            }
        }
        theta += XM_2PI / 16;
    }

    memcpy_s(m_pInstancedBuffer->MapDiscard(m_pd3dImmediateContext.Get()), m_pInstancedBuffer->GetByteWidth(),
        treeData.data(), treeData.size() * sizeof(BasicEffect::InstancedData));
    m_pInstancedBuffer->Unmap(m_pd3dImmediateContext.Get());
}
