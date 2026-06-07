#pragma once
#include "D3D11Application.h"
#include <SpriteBatch.h>
#include <memory>

using namespace DirectX;

class LaTaleDoujin : public D3D11Application
{
public:
    LaTaleDoujin();
    ~LaTaleDoujin();

    bool Init();

    void UpdateScene();
    void DrawScene();

private:
    std::unique_ptr<SpriteBatch> m_SpriteBatch;
    ComPtr<ID3D11ShaderResourceView> m_IrisTexture;
};
