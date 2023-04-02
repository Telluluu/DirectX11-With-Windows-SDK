#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include<Windows.h>
#include"Mouse.h"
#include"Keyboard.h"
#include<memory>

using namespace DirectX;


const D3D11_INPUT_ELEMENT_DESC GameApp::VertexPosColor::inputLayout[2] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight), m_CBuffer()
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
    //m_pMouse = std::make_unique<DirectX::Mouse>();
    //m_pKeyboard = std::make_unique<DirectX::Keyboard>();

    if (!D3DApp::Init())
        return false;

    if (!InitEffect())
        return false;

    if (!InitResource())
        return false;

    ////将鼠标初始化
    //m_pMouse->SetWindow(m_hMainWnd);
    //m_pMouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);

    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt) //dt为两帧间隔时间
{
    // ImGui内部示例窗口
    ImGui::ShowAboutWindow();
    ImGui::ShowDemoWindow();
    ImGui::ShowUserGuide();

    //static float phi = 0.0f, theta = 0.0f;
    //phi += 0.3f * dt, theta += 0.37f * dt;

    //static float pyramidPhi = 0.0f, pyramidTheta = 0.0f, pyramidmid = 1.0f; //记录旋转角度、缩放倍率
    //static float trans_pyramidPhi = 0.0f, trans_pyramidTheta = 0.0f, trans_pyramidDelta = 0.0f; //记录平移距离

    //Mouse::State mouseState = m_pMouse->GetState(); //用GetState()函数获取当前帧鼠标运动状态
    //Mouse::State lastMouseState = m_MouseTracker.GetLastState(); //获取前一帧鼠标状态
    //m_MouseTracker.Update(mouseState);//获取完鼠标状态，更新鼠标状态

    //Keyboard::State keyState = m_pKeyboard->GetState(); //获取当前帧下键盘所有按键的状态
    //Keyboard::State lastKeyState = m_KeyboardTracker.GetLastState(); //判断按键是刚按下还是刚放开
    //m_KeyboardTracker.Update(keyState);//获取完键盘状态，更新键盘状态


    //int dx = mouseState.x - lastMouseState.x, dy = mouseState.y - lastMouseState.y;//用dx和dy记录鼠标的x轴/y轴偏移量
    //int dscrollWheel = mouseState.scrollWheelValue - lastMouseState.scrollWheelValue;//用dmin来记录滚轮变化值

    ///******************************************************************************************************/
    ///*旋转控制*/

    ////左键按下且按住时
    //if (mouseState.leftButton == true && m_MouseTracker.leftButton == m_MouseTracker.HELD)
    //{
    //    // 旋转立方体
    //    pyramidTheta -= (dx) * 0.01f;
    //    pyramidPhi -= (dy) * 0.01f;
    //}

    ////根据按下的按键进行旋转
    //if (keyState.IsKeyDown(Keyboard::Up))  //WS键或上下键控制y轴
    //    pyramidPhi += dt * 2;
    //if (keyState.IsKeyDown(Keyboard::Down))
    //    pyramidPhi -= dt * 2;
    //if (keyState.IsKeyDown(Keyboard::Left))  //AD键或左右键控制x轴
    //    pyramidTheta += dt * 2;
    //if (keyState.IsKeyDown(Keyboard::Right))
    //    pyramidTheta -= dt * 2;

    ///******************************************************************************************************/

    ////缩放控制
    //pyramidmid += dscrollWheel * 0.001f;  //根据滚轮变化值来决定缩放
    //if (keyState.IsKeyDown(Keyboard::Q))  //QE键控制缩放
    //    pyramidmid += dt * 2;
    //if (keyState.IsKeyDown(Keyboard::E))
    //    pyramidmid -= dt * 2;

    ///******************************************************************************************************/
    ///*平移控制*/
    //    //右键按下且按住时
    //if (mouseState.rightButton == true && m_MouseTracker.rightButton == m_MouseTracker.HELD)
    //{
    //    // 平移立方体
    //    trans_pyramidTheta -= (dy) * 0.01f;
    //    trans_pyramidPhi += (dx) * 0.01f;
    //}
    ////根据按下的按键进行平移
    //if (keyState.IsKeyDown(Keyboard::W))  //WS键或上下键控制y轴
    //    trans_pyramidTheta += dt * 2;
    //if (keyState.IsKeyDown(Keyboard::S))
    //    trans_pyramidTheta -= dt * 2;
    //if (keyState.IsKeyDown(Keyboard::A))  //AD键或左右键控制x轴
    //    trans_pyramidPhi -= dt * 2;
    //if (keyState.IsKeyDown(Keyboard::D))
    //    trans_pyramidPhi += dt * 2;
    ///******************************************************************************************************/
    ////更新常量缓冲区
    ////更改world矩阵，先旋转再缩放，最后平移
    //m_CBuffer.world = XMMatrixTranspose(XMMatrixRotationY(pyramidTheta) * XMMatrixRotationX(pyramidPhi) * XMMatrixScaling(pyramidmid, pyramidmid, pyramidmid) * XMMatrixTranslation(trans_pyramidPhi, trans_pyramidTheta, trans_pyramidDelta));
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(m_CBuffer), &m_CBuffer, sizeof(m_CBuffer));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffer.Get(), 0);
}

void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);

    static float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };	// RGBA = (0,0,0,255)
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&black));
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // 绘制立方体
    m_pd3dImmediateContext->DrawIndexed(18, 0, 0);

    ImGui::Render();
    // 下面这句话会触发ImGui在Direct3D的绘制
    // 因此需要在此之前将后备缓冲区绑定到渲染管线上
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, 0));
}


bool GameApp::InitEffect()
{
    ComPtr<ID3DBlob> blob;

    // 创建顶点着色器
    HR(CreateShaderFromFile(L"HLSL\\Cube_VS.cso", L"HLSL\\Cube_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));
    // 创建顶点布局
    HR(m_pd3dDevice->CreateInputLayout(VertexPosColor::inputLayout, ARRAYSIZE(VertexPosColor::inputLayout),
        blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));

    // 创建像素着色器
    HR(CreateShaderFromFile(L"HLSL\\Cube_PS.cso", L"HLSL\\Cube_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));

    return true;
}

bool GameApp::InitResource()
{
    // ******************
    // 设置四棱锥顶点
    //           4
    //         / \\
    //        /   \\
    //     1 / ____\\3        很抽象的四棱锥，凑合看吧2333   
    //    | /     | /         底面四个顶点分别是0 1 2 3号
    //    |/______|/          棱锥顶点是4号
    //  0        2
    VertexPosColor vertices[] =
    {
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }
    };
    // 设置顶点缓冲区描述
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof vertices;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    // 新建顶点缓冲区
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;
    HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

    // ******************
    // 索引数组
    //
    DWORD indices[] = {
        // 正面
        0, 4, 2,
        // 左面
        1, 4, 0,
        // 背面
        3, 4, 1,
        // 右面
        2, 4, 3,
        // 底面
        0, 2, 1,
        1, 2, 3
    };
    // 设置索引缓冲区描述
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof indices;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    // 新建索引缓冲区
    InitData.pSysMem = indices;
    HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));
    // 输入装配阶段的索引缓冲区设置
    m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);


    // ******************
    // 设置常量缓冲区描述
    //
    D3D11_BUFFER_DESC cbd;
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(ConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    // 新建常量缓冲区，不使用初始数据
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffer.GetAddressOf()));


    // 初始化常量缓冲区的值
    // 如果你不熟悉这些矩阵，可以先忽略，待读完第四章后再回头尝试修改
    m_CBuffer.world = XMMatrixIdentity();	// 单位矩阵的转置是它本身
    m_CBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
        XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
        XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    ));
    m_CBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));


    // ******************
    // 给渲染管线各个阶段绑定好所需资源
    //

    // 输入装配阶段的顶点缓冲区设置
    UINT stride = sizeof(VertexPosColor);	// 跨越字节数
    UINT offset = 0;						// 起始偏移量

    m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    // 设置图元类型，设定输入布局
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());
    // 将着色器绑定到渲染管线
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
    // 将更新好的常量缓冲区绑定到顶点着色器
    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());

    m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

    // ******************
    // 设置调试对象名
    //
    D3D11SetDebugObjectName(m_pVertexLayout.Get(), "VertexPosColorLayout");
    D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
    D3D11SetDebugObjectName(m_pIndexBuffer.Get(), "IndexBuffer");
    D3D11SetDebugObjectName(m_pConstantBuffer.Get(), "ConstantBuffer");
    D3D11SetDebugObjectName(m_pVertexShader.Get(), "Cube_VS");
    D3D11SetDebugObjectName(m_pPixelShader.Get(), "Cube_PS");

    return true;
}
