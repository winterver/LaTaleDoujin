#pragma once
#include <d3d11_1.h>
#include "Utility.h"
#include <SimpleMath.h>
#include <vector>

using namespace DirectX::SimpleMath;

class DebugBatch
{
public:
    DebugBatch(ID3D11Device* device);

    void PutLine(Vector2 point1, Vector2 point2, Vector4 color = Vector4(1, 1, 1, 1), float depth = 0);
    void PutHollowRect(Vector2 position, Vector2 size, Vector4 color = Vector4(1, 1, 1, 1), float depth = 0);
    void PutSolidRect(Vector2 position, Vector2 size, Vector4 color = Vector4(1, 1, 1, 1), float depth = 0);
    void ClearScene();
    void UpdateScene();

    void DrawScene(Matrix transform = DirectX::XMMatrixIdentity());
    void DrawHollowSprite(Vector2 position, Vector2 size, Vector4 color = Vector4(1, 1, 1, 1), float depth = 0, Matrix transform = DirectX::XMMatrixIdentity());
    void DrawSolidSprite(Vector2 position, Vector2 size, Vector4 color = Vector4(1, 1, 1, 1), float depth = 0, Matrix transform = DirectX::XMMatrixIdentity());

private:
    void Prepare(Matrix transform, Vector4 color);

private:
    ComPtr<ID3D11Device> mDevice;
    ComPtr<ID3D11DeviceContext> mContext;
    ComPtr<ID3D11VertexShader> mVertexShader;
    ComPtr<ID3D11PixelShader> mPixelShader;
    ComPtr<ID3D11InputLayout> mLayout;

    struct { Matrix MVP; Vector4 Color; } mConstants;
    ComPtr<ID3D11Buffer> mConstantBuffer;

    std::vector<float> mLines;
    std::vector<float> mSolids;
    ComPtr<ID3D11Buffer> mLineBuffer;
    ComPtr<ID3D11Buffer> mSolidBuffer;
};

