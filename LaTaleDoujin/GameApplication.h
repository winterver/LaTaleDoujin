#pragma once
#include "D3D11Application.h"

class GameScene;

class GameApplication : public D3D11Application
{
public:
    GameApplication(const TCHAR* title, int width, int height);
    virtual ~GameApplication();

    SysTimer* GetTimer() { return &mTimer; }
    ID3D11Device* GetDevice() { return mDevice.Get(); }
    ID3D11DeviceContext* GetContext() { return mContext.Get(); }

protected:
    virtual void InitScene() override;
    virtual void UpdateScene() override;
    virtual void DrawScene() override;

    void ChangeScene(unique_ptr<GameScene> scene);

private:
    unique_ptr<GameScene> mCurrentScene;
    unique_ptr<GameScene> mNextScene;
};

