#pragma once
#include "D3D11Application.h"
#include <CommonStates.h>
#include <SpriteBatch.h>

using DirectX::CommonStates;
using DirectX::SpriteBatch;

class PhysicsSystem;
class Player;
class DebugBatch;

class LaTaleDoujin : public D3D11Application
{
public:
    LaTaleDoujin();
    ~LaTaleDoujin();

protected:
    void InitScene() override;
    void UpdateScene() override;
    void DrawScene() override;

private:
    unique_ptr<CommonStates> mCommonStates;
    unique_ptr<SpriteBatch> mSpriteBatch;
    ComPtr<ID3D11ShaderResourceView> mIrisTexture;

    unique_ptr<PhysicsSystem> mPhysicsSystem;
    unique_ptr<Player> mPlayer;
    unique_ptr<DebugBatch> mDebugBatch;
};
