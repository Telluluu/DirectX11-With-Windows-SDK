# 实现Imgui

## 构建imgui

#### 初始化ImGui

在d3dApp.h中包含头文件

```c++
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
```

##### 在D3DApp类中的protected声明InitImGui方法

##### 定义InitImGui

```c++
bool D3DApp::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // 允许键盘控制
    io.ConfigWindowsMoveFromTitleBarOnly = true;              // 仅允许标题拖动

    // 设置Dear ImGui风格
    ImGui::StyleColorsDark();
    
    // 设置平台/渲染器后端
    ImGui_ImplWin32_Init(m_hMainWnd);
    ImGui_ImplDX11_Init(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get());
    
    return true;

}
```

##### 在D3DApp::Init()中初始化

```c++
if (!InitImGui())
    return false;
```

##### 在d3dApp.cpp中声明对外部函数的引用

`extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);`

##### 在消息处理函数MsgProc函数中添加对ImGui的处理

```c++
if (ImGui_ImplWin32_WndProcHandler(m_hMainWnd, msg, wParam, lParam))
    return true;
```

##### 在D3DApp::Run中插入这三个函数，用于启动ImGui新一帧 的记录与绘制

```c++
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
```

##### 在DrawScene中绘制ImGui

```c++
    ImGui::Render();
    // 下面这句话会触发ImGui在Direct3D的绘制
    // 因此需要在此之前将后备缓冲区绑定到渲染管线上
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
```

##### 在UpdateScene中创建几个窗口测试一下

```c++
// ImGui内部示例窗口
ImGui::ShowAboutWindow();
ImGui::ShowDemoWindow();
ImGui::ShowUserGuide();
```

#### 结果

![](markdownfiles\ImGui构建.png)



## 实现其旋转缩放和移动

##### 用ImGuiIO来获取鼠标输入 

```c++
ImGuiIO& io = ImGui::GetIO();

io.MouseDelta; //当前帧鼠标位移量

io.MouseWheel;//鼠标滚轮
```

##### 之后用ImGui提供的函数来获取一些常用的键鼠事件，缩放旋转平移的实现和前一题用DXTK实现过程基本一样

```c++
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
        pyramidmid += io.MouseWheel * 0.02f;
        if (pyramidmid > 2.0f)//设置缩放速率阈值
            pyramidmid = 2.0f;
        else if (pyramidmid < 0.2f)
            pyramidmid = 0.2f;
    }
}
```

##### 最后更新下常量缓冲区

```c++
    //先缩放，再旋转，最后平移
    m_CBuffer.world = XMMatrixTranspose(
        XMMatrixScaling(pyramidmid, pyramidmid, pyramidmid)*       
        XMMatrixRotationY(pyramidTheta) * XMMatrixRotationX(pyramidPhi) *
        XMMatrixTranslation(trans_pyramidPhi, trans_pyramidTheta, trans_pyramidDelta));
    
	D3D11_MAPPED_SUBRESOURCE mappedData;
    HR(m_pd3dImmediateContext->Map(m_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
    memcpy_s(mappedData.pData, sizeof(m_CBuffer), &m_CBuffer, sizeof(m_CBuffer));
    m_pd3dImmediateContext->Unmap(m_pConstantBuffer.Get(), 0);
```

​	

## 实现文本显示

##### 在UpdateScene函数中用ImGui提供的函数实现

```c++
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
        /*四个参数分别为滑条名，滑条控制的变量，最左值及最右值*/
        ImGui::SliderFloat("Scale", &pyramidmid, 0.2f, 10.0f);//用一个滑条来控制缩放，并显示当前缩放倍率
        ImGui::SliderFloat("Phi", &pyramidPhi, -XM_PI, XM_PI);     // 显示转动角度
        ImGui::SliderFloat("Theta", &pyramidTheta, -XM_PI, XM_PI);  // 显示转动角度
    }
    //关闭窗口
    ImGui::End();
```

##### 结果

![](markdownfiles\ImGui鼠标操作及文本提示.jpg)
