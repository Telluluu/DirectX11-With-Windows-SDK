# D3D11SetDebugObjectName函数报错

Debug模式下d3dUtil.h中

`void D3D11SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_ const char(&name)[TNameLength])`函数报错

![](https://raw.githubusercontent.com/Telluluu/DirectX11-With-Windows-SDK/%E5%88%98%E6%9B%A6%E7%83%A8/%E4%BD%9C%E4%B8%9A%E6%B5%81%E7%A8%8B%E8%AF%B4%E6%98%8E%E5%8F%8A%E9%97%AE%E7%AD%94%E9%A2%98/markdownfiles/D3D11SetDebugObjectName%E5%87%BD%E6%95%B0%E6%8A%A5%E9%94%99.png)

### 原因：

在设置调试对象名时传入了一个空指针，导致resource为空

![](https://raw.githubusercontent.com/Telluluu/DirectX11-With-Windows-SDK/%E5%88%98%E6%9B%A6%E7%83%A8/%E4%BD%9C%E4%B8%9A%E6%B5%81%E7%A8%8B%E8%AF%B4%E6%98%8E%E5%8F%8A%E9%97%AE%E7%AD%94%E9%A2%98/markdownfiles/D3D11SetDebugObjectName%E4%B8%AD%E4%BC%A0%E5%85%A5%E4%BA%86%E4%B8%80%E4%B8%AA%E7%A9%BA%E6%8C%87%E9%92%88.png)

### 解决方法：

检查InitEffect中是否创建了该项

因为本项目中并未用到2D顶点布局，没有创建导致传入了空指针

若需要2D顶点布局，在InitEffect中创建即可

```c++
HR(m_pd3dDevice->CreateInputLayout(VertexPosTex::inputLayout, ARRAYSIZE(VertexPosTex::inputLayout),
    blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout2D.GetAddressOf()));
```