#pragma once
#include "D3D11Application.h"

class GameScene;

class GameApplication : public D3D11Application
{
public:
    GameApplication(const TCHAR* title, int width, int height);
    virtual ~GameApplication();

    PROPERTY(Timer, &mTimer)
    PROPERTY(Device, mDevice.Get())
    PROPERTY(Context, mContext.Get())
    PROPERTY(SwapChain, mSwapChain.Get())
    PROPERTY(RenderTargetView, mRenderTargetView.Get())
    PROPERTY(DepthStencilView, mDepthStencilView.Get())

protected:
    virtual void InitScene() override;
    virtual void UpdateScene() override;
    virtual void DrawScene() override;

    void ChangeScene(unique_ptr<GameScene> scene);

private:
    unique_ptr<GameScene> mCurrentScene;
    unique_ptr<GameScene> mNextScene;
};

