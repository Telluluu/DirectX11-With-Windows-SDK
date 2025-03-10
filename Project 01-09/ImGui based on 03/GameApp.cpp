#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include<Windows.h>
#include"Mouse.h"
#include"Keyboard.h"
#include<memory>
#include "d3dApp.h"

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

    static float pyramidPhi = 0.0f, pyramidTheta = 0.0f, pyramidmid = 1.0f; //记录旋转角度、缩放倍率
    static float trans_pyramidPhi = 0.0f, trans_pyramidTheta = 0.0f, trans_pyramidDelta = 0.0f; //记录平移距离

    if (ImGui::Begin("Use ImGui"))//创建一个名为"Use ImGui"的窗口
    {
        //用Text函数打印操作提示
        ImGui::Text("Hold Left Button to Rotate");
        ImGui::Text("Hold Right Button to Translate");

        //使用按钮来重置缩放、旋转、平移
        if (ImGui::Button("Reset Params"))
        {
            //旋转角度、缩放倍率、平移距离初始化
            pyramidPhi = pyramidTheta = 0.0f;
            pyramidmid = 1.0f;
            trans_pyramidPhi = trans_pyramidTheta = trans_pyramidDelta = 0.0f;
        }
        ImGui::SliderFloat("Scale", &pyramidmid, 0.2f, 10.0f);//用一个滑条来控制缩放，并显示当前缩放倍率
        ImGui::SliderFloat("Phi", &pyramidPhi, -XM_PI, XM_PI);     // 显示转动角度
        ImGui::SliderFloat("Theta", &pyramidTheta, -XM_PI, XM_PI);  // 显示转动角度
    }
    //关闭窗口
    ImGui::End();

    ImGuiIO& io = ImGui::GetIO();//获取鼠标输入
    //auto& delta = io.MouseDelta; // 当前帧鼠标位移量
    //io.MouseWheel;//鼠标滚轮

    //int dx = , dy = ;//用dx和dy记录鼠标的x轴/y轴偏移量
    //int dscrollWheel = ;//用dmin来记录滚轮变化值

    /******************************************************************************************************/
    if (!ImGui::IsAnyItemActive())//当ImGui的UI处于非活跃状态时
    {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))//拖动右键平移
        {
            trans_pyramidPhi += io.MouseDelta.x * 0.01f;
            trans_pyramidTheta -= io.MouseDelta.y * 0.01f;
        }
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))//拖动左键旋转
        {
            pyramidPhi -= io.MouseDelta.y * 0.01f;
            pyramidTheta -= io.MouseDelta.x * 0.01f;
        }
        if (io.MouseWheel!=0.0f)
        {
            pyramidmid += io.MouseWheel * 0.2f;
            if (pyramidmid > 10.0f)//设置缩放速率阈值
                pyramidmid = 10.0f;
            else if (pyramidmid < 0.2f)
                pyramidmid = 0.2f;
        }
    }
    /******************************************************************************************************/
    //更新常量缓冲区
    //先缩放，再旋转，最后平移
    m_CBuffer.world = XMMatrixTranspose(XMMatrixScaling(pyramidmid, pyramidmid, pyramidmid)*       
        XMMatrixRotationY(pyramidTheta) * XMMatrixRotationX(pyramidPhi) *
        XMMatrixTranslation(trans_pyramidPhi, trans_pyramidTheta, trans_pyramidDelta));
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
