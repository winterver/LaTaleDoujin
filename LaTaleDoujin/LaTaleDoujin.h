#pragma once
#include "D3D11Application.h"
#include <memory>

namespace DirectX
{
    inline namespace DX11
    {
        class CommonStates;
        class SpriteBatch;
    }
    class Keyboard;
    class Mouse;
}

using DirectX::CommonStates;
using DirectX::SpriteBatch;
using DirectX::Keyboard;
using DirectX::Mouse;

class DebugBatch;

class LaTaleDoujin : public D3D11Application
{
public:
    LaTaleDoujin();
    ~LaTaleDoujin();

    bool Init();

protected:
    void UpdateScene();
    void DrawScene();

    LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    std::unique_ptr<Keyboard> m_Keyboard;
    std::unique_ptr<Mouse> m_Mouse;

    std::unique_ptr<CommonStates> m_CommonStates;
    std::unique_ptr<SpriteBatch> m_SpriteBatch;
    ComPtr<ID3D11ShaderResourceView> m_IrisTexture;

    std::unique_ptr<DebugBatch> m_DebugBatch;
};
