#pragma once
#include "D3D11Application.h"
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <CommonStates.h>
#include <memory>

using DirectX::CommonStates;
using DirectX::SpriteBatch;
using DirectX::Keyboard;
using DirectX::Mouse;
using Tracker = Keyboard::KeyboardStateTracker;
using Keys = Keyboard::Keys;

class PhysicsSystem;
class Player;
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
    std::unique_ptr<Tracker> m_Tracker;
    std::unique_ptr<Mouse> m_Mouse;

    std::unique_ptr<CommonStates> m_CommonStates;
    std::unique_ptr<SpriteBatch> m_SpriteBatch;
    ComPtr<ID3D11ShaderResourceView> m_IrisTexture;

    std::unique_ptr<PhysicsSystem> m_PhysicsSystem;
    std::unique_ptr<Player> m_Player;
    std::unique_ptr<DebugBatch> m_DebugBatch;
};
