#pragma once
#include "GameApplication.h"
#include <CommonStates.h>
#include <SpriteBatch.h>

class GameScene
{
    using CommonStates = DirectX::CommonStates;
    using SpriteBatch = DirectX::SpriteBatch;
    friend class GameApplication;

public:
    GameScene(GameApplication* game);
    virtual ~GameScene();

protected:
    virtual void InitScene();
    virtual void UpdateScene();
    virtual void DrawScene();

protected:
    GameApplication* mGame;
    PROPERTY(Timer, mGame->Timer())
    PROPERTY(Device, mGame->Device())
    PROPERTY(Context, mGame->Context())
    PROPERTY(SwapChain, mGame->SwapChain())
    PROPERTY(RenderTargetView, mGame->RenderTargetView())
    PROPERTY(DepthStencilView, mGame->DepthStencilView())

    unique_ptr<CommonStates> mCommonStates;
    unique_ptr<SpriteBatch> mSpriteBatch;
};

