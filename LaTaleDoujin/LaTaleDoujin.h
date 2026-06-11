#pragma once
#include "D3D11Application.h"
#include <SpriteBatch.h>
#include <memory>

using namespace DirectX;

class LaTaleDoujin : public D3D11Application
{
public:
    LaTaleDoujin();
    ~LaTaleDoujin();

    bool Init();

protected:
    void OnResize();
    void UpdateScene();
    void DrawScene();

private:
    ComPtr<ID3D11Texture2D> m_Framebuffer1;
    ComPtr<ID3D11Texture2D> m_Framebuffer2;
    ComPtr<ID3D11RenderTargetView> m_Framebuffer1View;
    ComPtr<ID3D11RenderTargetView> m_Framebuffer2View;
    ComPtr<ID3D11ShaderResourceView> m_Framebuffer1Tex;
    ComPtr<ID3D11ShaderResourceView> m_Framebuffer2Tex;

    std::unique_ptr<SpriteBatch> m_SpriteBatch;
    ComPtr<ID3D11PixelShader> m_GaussianBlur;
    ComPtr<ID3D11Buffer> m_GaussianParametersH;
    ComPtr<ID3D11Buffer> m_GaussianParametersV;
    ComPtr<ID3D11ShaderResourceView> m_IrisTexture;
};
