#pragma once
#include "D3D11Application1.h"
#include <CommonStates.h>
#include <SpriteBatch.h>
#include "Utility.h"
#include <memory>

using DirectX::CommonStates;
using DirectX::SpriteBatch;

class PhysicsSystem;
class Player;
class DebugBatch;

class LaTaleDoujin : public D3D11Application1
{
public:
    LaTaleDoujin();
    ~LaTaleDoujin();

protected:
    void InitScene() override;
    void UpdateScene() override;
    void DrawScene() override;

private:
    std::unique_ptr<CommonStates> m_CommonStates;
    std::unique_ptr<SpriteBatch> m_SpriteBatch;
    ComPtr<ID3D11ShaderResourceView> m_IrisTexture;

    std::unique_ptr<PhysicsSystem> m_PhysicsSystem;
    std::unique_ptr<Player> m_Player;
    std::unique_ptr<DebugBatch> m_DebugBatch;
};
