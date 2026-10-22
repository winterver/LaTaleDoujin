#pragma once
#include "GameApplication.h"
#include <CommonStates.h>
#include <SpriteBatch.h>
#include <functional>
#include <vector>

class GameScene
{
    using CommonStates = DirectX::CommonStates;
    using SpriteBatch = DirectX::SpriteBatch;
    friend class GameApplication;

public:
    GameScene(GameApplication* game);
    virtual ~GameScene();

    void StepInit();
    bool IsInitIncomplete();
    float GetInitProgress();

    void LoadTexture(ID3D11ShaderResourceView** texture, const TCHAR* path, bool mipmap = false);

protected:
    virtual void InitScene();
    virtual void UpdateScene();
    virtual void DrawScene();

protected:
    GameApplication* mGame;
    SysTimer* mTimer;
    ID3D11Device* mDevice;
    ID3D11DeviceContext* mContext;
    IDXGISwapChain* mSwapChain;
    ID3D11RenderTargetView* mRenderTargetView;
    ID3D11DepthStencilView* mDepthStencilView;

    unique_ptr<CommonStates> mCommonStates;
    unique_ptr<SpriteBatch> mSpriteBatch;

private:
    enum InitStatus;
    InitStatus mStatus;

    std::vector<std::function<void()>> mTasks;
    decltype(mTasks)::iterator mIndex;
};

