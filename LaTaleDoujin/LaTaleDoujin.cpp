#include "LaTaleDoujin.h"

LaTaleDoujin::LaTaleDoujin()
    : D3D11Application(L"La Tale Doujin", 1360, 768)
{
}

LaTaleDoujin::~LaTaleDoujin() = default;

void LaTaleDoujin::UpdateScene()
{
}

void LaTaleDoujin::DrawScene()
{
    static float cornflowerblue[4] = { 100.0f/255, 149.0f/255, 237.0f/255, 1.0f };
    m_pContext->ClearRenderTargetView(m_pRenderTargetView.Get(), cornflowerblue);
    m_pContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_pSwapChain->Present(0, 0);
}

