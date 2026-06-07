#include "LaTaleDoujin.h"
#include "DXHelper.h"

void CreateTextureFromFile(
    ID3D11Device* pDevice,
    _In_opt_ ID3D11DeviceContext* pContext,
    const WCHAR* szFileName,
    _Outptr_ ID3D11ShaderResourceView** textureView);

LaTaleDoujin::LaTaleDoujin()
    : D3D11Application(L"La Tale Doujin", 1360, 768)
{
}

LaTaleDoujin::~LaTaleDoujin() = default;

bool LaTaleDoujin::Init()
{
    bool val = D3D11Application::Init();

    m_SpriteBatch = std::make_unique<DirectX::SpriteBatch>(m_pContext.Get());

    ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    CreateTextureFromFile(m_pDevice.Get(), nullptr, L"C:/Data/Develop/LaTaleDoujin_Sharp/LaTaleDoujin/resources/IRIS.PNG", &m_IrisTexture);

    return val;
}

void LaTaleDoujin::UpdateScene()
{
}

void LaTaleDoujin::DrawScene()
{
    static float cornflowerblue[] = { 100.0f/255, 149.0f/255, 237.0f/255, 1.0f };
    m_pContext->ClearRenderTargetView(m_pRenderTargetView.Get(), cornflowerblue);
    m_pContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_SpriteBatch->SetViewport(m_Viewport);
    m_SpriteBatch->Begin();
    m_SpriteBatch->Draw(m_IrisTexture.Get(), XMFLOAT2());
    m_SpriteBatch->End();

    m_pSwapChain->Present(0, 0);
}

