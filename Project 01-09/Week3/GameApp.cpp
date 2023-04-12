#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"

#include"Mouse.h"
#include"Keyboard.h"
#include<memory>
#include "d3dApp.h"
#include"Vertex.h"

using namespace DirectX;


//const D3D11_INPUT_ELEMENT_DESC GameApp::VertexPosColor::inputLayout[2] = {
//    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
//};

GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight),
    m_IndexCount(),
    m_CurrFrame(),
    m_CurrMode(ShowMode::WoodCrate),
    m_VSConstantBuffer(),
    m_PSConstantBuffer()
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
    m_pMouse = std::make_unique<DirectX::Mouse>();
    m_pKeyboard = std::make_unique<DirectX::Keyboard>();

    if (!D3DApp::Init())
        return false;

    if (!InitEffect())
        return false;

    if (!InitResource())
        return false;

    //将鼠标初始化
    m_pMouse->SetWindow(m_hMainWnd);
    m_pMouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);

    return true;
}

void GameApp::OnResize()
{
    D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt) //dt为两帧间隔时间
{
    Keyboard::State state = m_pKeyboard->GetState();
    m_KeyboardTracker.Update(state);

    // 播放木箱动画
    m_CurrMode = ShowMode::WoodCrate;
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout3D.Get());
    auto meshData = Geometry::CreateBox();
    ResetMesh(meshData);
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader3D.Get(), nullptr, 0);
    m_pd3dImmediateContext->PSSetShader(m_pPixelShader3D.Get(), nullptr, 0);
    m_pd3dImmediateContext->PSSetShaderResources(0, 1, m_pWoodCrate.GetAddressOf());

    ImGui::Render();
    //m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout2D.Get());
    //auto meshData = Geometry::Create2DShow();
    //ResetMesh(meshData);
    //m_pd3dImmediateContext->VSSetShader(m_pVertexShader2D.Get(), nullptr, 0);
    //m_pd3dImmediateContext->PSSetShader(m_pPixelShader2D.Get(), nullptr, 0);
    //m_pd3dImmediateContext->PSSetShaderResources(0, 1, m_pFireAnims[0].GetAddressOf());

    static float phi = 0.0f, theta = 0.0f;
    phi += 0.0001f, theta += 0.00015f;
    XMMATRIX W = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
    m_VSConstantBuffer.world = XMMatrixTranspose(W);
    m_VSConstantBuffer.worldInvTranspose = XMMatrixTranspose(InverseTranspose(W));
 
    static float tex_phi = 0.0f;
    tex_phi -= 0.001f;
    XMMATRIX tex_Matrix = XMMatrixTranslation(-0.5f, -0.5f, 0.0f) * XMMatrixRotationZ(tex_phi) * XMMatrixTranslation(0.5f, 0.5f, 0.0f);
    m_VSConstantBuffer.tex_rotation = XMMatrixTranspose(tex_Matrix);

    /******************************************************************************************************/
    //更新常量缓冲区
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(VSConstantBuffer), &m_VSConstantBuffer, sizeof(VSConstantBuffer));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffers[0].Get(), 0);
}

void GameApp::DrawScene()
{
    assert(m_pd3dImmediateContext);
    assert(m_pSwapChain);

    static float black[4] = { 0.0f, 0.0f, 1.0f, 1.0f };	// RGBA = (0,0,0,255) //设置背景颜色
    m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&black));
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // 绘制立方体
    m_pd3dImmediateContext->DrawIndexed(m_IndexCount, 0, 0);

    //ImGui::Render();
    // 下面这句话会触发ImGui在Direct3D的绘制
    // 因此需要在此之前将后备缓冲区绑定到渲染管线上
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, 0));
}


bool GameApp::InitEffect()
{
    ComPtr<ID3DBlob> blob;

    // 创建顶点着色器(2D)
    HR(CreateShaderFromFile(L"HLSL\\Basic_VS_2D.cso", L"HLSL\\Basic_VS_2D.hlsl", "VS_2D", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader2D.GetAddressOf()));
    // 创建顶点布局(2D)
    HR(m_pd3dDevice->CreateInputLayout(VertexPosTex::inputLayout, ARRAYSIZE(VertexPosTex::inputLayout),
        blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout2D.GetAddressOf()));

    // 创建像素着色器(2D)
    HR(CreateShaderFromFile(L"HLSL\\Basic_PS_2D.cso", L"HLSL\\Basic_PS_2D.hlsl", "PS_2D", "ps_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader2D.GetAddressOf()));

    // 创建顶点着色器(3D)
    HR(CreateShaderFromFile(L"HLSL\\Basic_VS_3D.cso", L"HLSL\\Basic_VS_3D.hlsl", "VS_3D", "vs_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader3D.GetAddressOf()));
    // 创建顶点布局(3D)
    HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalTex::inputLayout, ARRAYSIZE(VertexPosNormalTex::inputLayout),
        blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout3D.GetAddressOf()));

    // 创建像素着色器(3D)
    HR(CreateShaderFromFile(L"HLSL\\Basic_PS_3D.cso", L"HLSL\\Basic_PS_3D.hlsl", "PS_3D", "ps_5_0", blob.ReleaseAndGetAddressOf()));
    HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader3D.GetAddressOf()));

    return true;
}

bool GameApp::InitResource()
{
    // 初始化网格模型并设置到输入装配阶段
    auto meshData = Geometry::CreateBox();  //调用Geometry中的CreateBox函数绘制一个立方体
    ResetMesh(meshData);

    //// ******************
    //// 设置立方体顶点
    ////    5________ 6
    ////    /|      /|
    ////   /_|_____/ |
    ////  1|4|_ _ 2|_|7
    ////   | /     | /
    ////   |/______|/
    ////  0       3
    //VertexPosColor vertices[] =
    //{
    //    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
    //    { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
    //    { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
    //    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
    //    { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
    //    { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
    //    { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
    //    { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) }
    //};
    //// 设置顶点缓冲区描述
    //D3D11_BUFFER_DESC vbd;
    //ZeroMemory(&vbd, sizeof(vbd));
    //vbd.Usage = D3D11_USAGE_IMMUTABLE;
    //vbd.ByteWidth = sizeof vertices;
    //vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    //vbd.CPUAccessFlags = 0;
    //// 新建顶点缓冲区
    //D3D11_SUBRESOURCE_DATA InitData;
    //ZeroMemory(&InitData, sizeof(InitData));
    //InitData.pSysMem = vertices;
    //HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));
    //// ******************
    //// 索引数组
    ////
    //DWORD indices[] = {
    //    // 正面
    //   0, 1, 2,
    //   2, 3, 0,
    //   // 左面
    //   4, 5, 1,
    //   1, 0, 4,
    //   // 顶面
    //   1, 5, 6,
    //   6, 2, 1,
    //   // 背面
    //   7, 6, 5,
    //   5, 4, 7,
    //   // 右面
    //   3, 2, 6,
    //   6, 7, 3,
    //   // 底面
    //   4, 0, 3,
    //   3, 7, 4  
    //};
    //// 设置索引缓冲区描述
    //D3D11_BUFFER_DESC ibd;
    //ZeroMemory(&ibd, sizeof(ibd));
    //ibd.Usage = D3D11_USAGE_IMMUTABLE;
    //ibd.ByteWidth = sizeof indices;
    //ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    //ibd.CPUAccessFlags = 0;
    //// 新建索引缓冲区
    //InitData.pSysMem = indices;
    //HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));
    //// 输入装配阶段的索引缓冲区设置
    //m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);


    // ******************
    // 设置常量缓冲区描述
    //
    D3D11_BUFFER_DESC cbd;
    ZeroMemory(&cbd, sizeof(cbd));
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(VSConstantBuffer);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    // 新建VS和PS的常量缓冲区
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[0].GetAddressOf()));
    cbd.ByteWidth = sizeof(PSConstantBuffer);
    HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[1].GetAddressOf()));

    // ******************
   // 初始化纹理和采样器状态

   // 初始化木箱纹理
    HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"..\\Texture\\WoodCrate.dds", nullptr, m_pWoodCrate.GetAddressOf()));

    // 初始化采样器状态
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    HR(m_pd3dDevice->CreateSamplerState(&sampDesc, m_pSamplerState.GetAddressOf()));

    // 初始化常量缓冲区的值
    // 如果你不熟悉这些矩阵，可以先忽略，待读完第四章后再回头尝试修改
    m_VSConstantBuffer.world = XMMatrixIdentity();	// 单位矩阵的转置是它本身
    m_VSConstantBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
        XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
        XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    ));
    m_VSConstantBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));
    m_VSConstantBuffer.worldInvTranspose = XMMatrixIdentity();

    // 初始化用于PS的常量缓冲区的值
    // 这里只使用一盏点光来演示
    m_PSConstantBuffer.pointLight[0].position = XMFLOAT3(0.0f, 0.0f, -10.0f);
    m_PSConstantBuffer.pointLight[0].ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    m_PSConstantBuffer.pointLight[0].diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    m_PSConstantBuffer.pointLight[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_PSConstantBuffer.pointLight[0].att = XMFLOAT3(0.0f, 0.1f, 0.0f);
    m_PSConstantBuffer.pointLight[0].range = 25.0f;

    m_PSConstantBuffer.dirLight[0].ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    m_PSConstantBuffer.dirLight[0].diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    m_PSConstantBuffer.dirLight[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_PSConstantBuffer.dirLight[0].direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);

    m_PSConstantBuffer.spotLight[0].position = XMFLOAT3(0.0f, 0.0f, -5.0f);
    m_PSConstantBuffer.spotLight[0].direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
    m_PSConstantBuffer.spotLight[0].ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_PSConstantBuffer.spotLight[0].diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_PSConstantBuffer.spotLight[0].specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_PSConstantBuffer.spotLight[0].att = XMFLOAT3(1.0f, 0.0f, 0.0f);
    m_PSConstantBuffer.spotLight[0].spot = 12.0f;
    m_PSConstantBuffer.spotLight[0].range = 10000.0f;

    m_PSConstantBuffer.numDirLight = 0;
    m_PSConstantBuffer.numPointLight = 1;
    m_PSConstantBuffer.numSpotLight = 0;
    // 初始化材质
    m_PSConstantBuffer.material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_PSConstantBuffer.material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_PSConstantBuffer.material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
    // 注意不要忘记设置此处的观察位置，否则高亮部分会有问题
    m_PSConstantBuffer.eyePos = XMFLOAT4(0.0f, 0.0f, -5.0f, 0.0f);

    // 更新PS常量缓冲区资源
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);



    // ******************
    // 给渲染管线各个阶段绑定好所需资源
    //

    //// 输入装配阶段的顶点缓冲区设置
    //UINT stride = sizeof(VertexPosColor);	// 跨越字节数
    //UINT offset = 0;						// 起始偏移量

    //m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    // 设置图元类型，设定输入布局
    m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout3D.Get());
    // 将着色器绑定到渲染管线
    m_pd3dImmediateContext->VSSetShader(m_pVertexShader3D.Get(), nullptr, 0);
    // 将更新好的常量缓冲区绑定到顶点着色器
    m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
    // PS常量缓冲区对应HLSL寄存于b1的常量缓冲区
    m_pd3dImmediateContext->PSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
    // 像素着色阶段设置好采样器
    m_pd3dImmediateContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());
    m_pd3dImmediateContext->PSSetShaderResources(0, 1, m_pWoodCrate.GetAddressOf());
    m_pd3dImmediateContext->PSSetShader(m_pPixelShader3D.Get(), nullptr, 0);
   

    // ******************
    // 设置调试对象名
    //
    D3D11SetDebugObjectName(m_pVertexLayout2D.Get(), "VertexPosTexLayout");
    D3D11SetDebugObjectName(m_pVertexLayout3D.Get(), "VertexPosNormalTexLayout");
    D3D11SetDebugObjectName(m_pConstantBuffers[0].Get(), "VSConstantBuffer");
    D3D11SetDebugObjectName(m_pConstantBuffers[1].Get(), "PSConstantBuffer");
    D3D11SetDebugObjectName(m_pVertexShader2D.Get(), "Basic_VS_2D");
    D3D11SetDebugObjectName(m_pVertexShader3D.Get(), "Basic_VS_3D");
    D3D11SetDebugObjectName(m_pPixelShader2D.Get(), "Basic_PS_2D");
    D3D11SetDebugObjectName(m_pPixelShader3D.Get(), "Basic_PS_3D");
    D3D11SetDebugObjectName(m_pSamplerState.Get(), "SSLinearWrap");

    return true;
}

template<class VertexType>
bool GameApp::ResetMesh(const Geometry::MeshData<VertexType>& meshData)
{
    // 释放旧资源
    m_pVertexBuffer.Reset();
    m_pIndexBuffer.Reset();



    // 设置顶点缓冲区描述
    D3D11_BUFFER_DESC vbd;
    ZeroMemory(&vbd, sizeof(vbd));
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = (UINT)meshData.vertexVec.size() * sizeof(VertexType);
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    // 新建顶点缓冲区
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = meshData.vertexVec.data();
    HR(m_pd3dDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.ReleaseAndGetAddressOf()));

    // 输入装配阶段的顶点缓冲区设置
    UINT stride = sizeof(VertexType);			// 跨越字节数
    UINT offset = 0;							// 起始偏移量

    m_pd3dImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);



    // 设置索引缓冲区描述
    m_IndexCount = (UINT)meshData.indexVec.size();
    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(DWORD) * m_IndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    // 新建索引缓冲区
    InitData.pSysMem = meshData.indexVec.data();
    HR(m_pd3dDevice->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.ReleaseAndGetAddressOf()));
    // 输入装配阶段的索引缓冲区设置
    m_pd3dImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);



    // 设置调试对象名
    D3D11SetDebugObjectName(m_pVertexBuffer.Get(), "VertexBuffer");
    D3D11SetDebugObjectName(m_pIndexBuffer.Get(), "IndexBuffer");

    return true;
}