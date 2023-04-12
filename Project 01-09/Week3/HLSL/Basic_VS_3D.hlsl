#include "Basic.hlsli"

// 顶点着色器(3D)
VertexPosHWNormalTex VS_3D(VertexPosNormalTex vIn)
{
    VertexPosHWNormalTex vOut; //定义传出顶点属性
    matrix viewProj = mul(g_View, g_Proj); //视图投影矩阵
    float4 posW = mul(float4(vIn.posL, 1.0f), g_World); //计算顶点的世界坐标

    vOut.posH = mul(posW, viewProj); //计算顶点MVP变换后的齐次坐标
    vOut.posW = posW.xyz; //保存世界坐标
    vOut.normalW = mul(vIn.normalL, (float3x3) g_WorldInvTranspose); //法线变换
    vOut.tex = mul(float4(vIn.tex,0.0f,1.0f), (float4x4) g_tex_rotation); // 纹理坐标，我们要让纹理旋转，所以乘于一个旋转矩阵
                                                                          //注意要将纹理坐标从xy转为xyzw,因为是2D纹理所以z轴坐标不影响效果
    return vOut;
}
