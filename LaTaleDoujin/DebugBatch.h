#pragma once
#include <d3d11_1.h>
#include <wrl/client.h>
#include <SimpleMath.h>
#include <vector>

using Microsoft::WRL::ComPtr;

class DebugBatch
{
    using Vector2 = DirectX::SimpleMath::Vector2;
    using Vector3 = DirectX::SimpleMath::Vector3;
    using Vector4 = DirectX::SimpleMath::Vector4;
    using Matrix = DirectX::SimpleMath::Matrix;

public:
    DebugBatch(ID3D11Device* device);

    void PutLine(Vector2 point1, Vector2 point2, Vector4 color = Vector4(1, 1, 1, 1), float depth = 0);
    void PutHollowRect(Vector2 position, Vector2 size, Vector4 color = Vector4(1, 1, 1, 1), float depth = 0);
    void PutSolidRect(Vector2 position, Vector2 size, Vector4 color = Vector4(1, 1, 1, 1), float depth = 0);
    void Update();

    void Draw(Matrix transform = DirectX::XMMatrixIdentity());

private:
    ComPtr<ID3D11Device> m_pDevice;
    ComPtr<ID3D11DeviceContext> m_pContext;
    ComPtr<ID3DBlob> m_VertexBlob;
    ComPtr<ID3DBlob> m_PixelBlob;
    ComPtr<ID3D11VertexShader> m_VertexShader;
    ComPtr<ID3D11PixelShader> m_PixelShader;
    ComPtr<ID3D11InputLayout> m_Layout;
    ComPtr<ID3D11Buffer> m_LineBuffer;
    ComPtr<ID3D11Buffer> m_SolidBuffer;
    ComPtr<ID3D11Buffer> m_MVPBuffer;

    std::vector<float> m_Lines;
    std::vector<float> m_Solids;
};

