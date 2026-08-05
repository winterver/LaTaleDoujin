#include "DebugBatch.h"
#include "Utility.h"
#include <d3dcompiler.h>

using namespace DirectX;

DebugBatch::DebugBatch(ID3D11Device* device)
{
    ComPtr<ID3DBlob> vertexBlob;
    ComPtr<ID3DBlob> pixelBlob;

    const char source[] = R"(
        struct PSInput
        {
            float4 Position : SV_POSITION;
            float4 Color : COLOR;
        };

        cbuffer Constants : register(b0)
        {
            matrix MVP;
            float4 Color;
        }

        PSInput VSMain(float3 position : POSITION, float4 color : COLOR)
        {
            PSInput result;

            result.Position = mul(float4(position, 1), MVP);
            result.Color = color * Color;

            return result;
        }

        float4 PSMain(PSInput input) : SV_TARGET
        {
            return input.Color;
        }
    )";

    const D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    CD3D11_BUFFER_DESC bufferDesc(sizeof(m_Constants), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    m_pDevice = device;
    m_pDevice->GetImmediateContext(&m_pContext);

    ThrowIfFailed(D3DCompile(source, sizeof(source), nullptr, nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexBlob, nullptr));
    ThrowIfFailed(D3DCompile(source, sizeof(source), nullptr, nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelBlob, nullptr));
    ThrowIfFailed(device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, &m_VertexShader));
    ThrowIfFailed(device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, &m_PixelShader));
    ThrowIfFailed(device->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &m_Layout));
    ThrowIfFailed(device->CreateBuffer(&bufferDesc, nullptr, &m_ConstantBuffer));

    PutHollowRect(Vector2(0, 0), Vector2(1, 1));
    PutSolidRect(Vector2(0, 0), Vector2(1, 1));
    UpdateScene();
}

void DebugBatch::PutLine(Vector2 point1, Vector2 point2, Vector4 color, float depth)
{
    m_Lines.push_back(point1.x); m_Lines.push_back(point1.y); m_Lines.push_back(depth);
    m_Lines.push_back(color.x); m_Lines.push_back(color.y); m_Lines.push_back(color.z); m_Lines.push_back(color.w);
    m_Lines.push_back(point2.x); m_Lines.push_back(point2.y); m_Lines.push_back(depth);
    m_Lines.push_back(color.x); m_Lines.push_back(color.y); m_Lines.push_back(color.z); m_Lines.push_back(color.w);
}

void DebugBatch::PutHollowRect(Vector2 position, Vector2 size, Vector4 color, float depth)
{
    PutLine(position, position+Vector2(size.x, 0), color, depth);
    PutLine(position+Vector2(size.x, 0), position+size, color, depth);
    PutLine(position+size, position+Vector2(0, size.y), color, depth);
    PutLine(position+Vector2(0, size.y), position, color, depth);
}

void DebugBatch::PutSolidRect(Vector2 position, Vector2 size, Vector4 color, float depth)
{
    m_Solids.push_back(position.x); m_Solids.push_back(position.y); m_Solids.push_back(depth);
    m_Solids.push_back(color.x); m_Solids.push_back(color.y); m_Solids.push_back(color.z); m_Solids.push_back(color.w);
    m_Solids.push_back(position.x+size.x); m_Solids.push_back(position.y); m_Solids.push_back(depth);
    m_Solids.push_back(color.x); m_Solids.push_back(color.y); m_Solids.push_back(color.z); m_Solids.push_back(color.w);
    m_Solids.push_back(position.x+size.x); m_Solids.push_back(position.y+size.y); m_Solids.push_back(depth);
    m_Solids.push_back(color.x); m_Solids.push_back(color.y); m_Solids.push_back(color.z); m_Solids.push_back(color.w);

    m_Solids.push_back(position.x+size.x); m_Solids.push_back(position.y+size.y); m_Solids.push_back(depth);
    m_Solids.push_back(color.x); m_Solids.push_back(color.y); m_Solids.push_back(color.z); m_Solids.push_back(color.w);
    m_Solids.push_back(position.x); m_Solids.push_back(position.y+size.y); m_Solids.push_back(depth);
    m_Solids.push_back(color.x); m_Solids.push_back(color.y); m_Solids.push_back(color.z); m_Solids.push_back(color.w);
    m_Solids.push_back(position.x); m_Solids.push_back(position.y); m_Solids.push_back(depth);
    m_Solids.push_back(color.x); m_Solids.push_back(color.y); m_Solids.push_back(color.z); m_Solids.push_back(color.w);
}

void DebugBatch::ClearScene()
{
    m_LineBuffer.Reset();
    m_SolidBuffer.Reset();
    m_Lines.clear();
    m_Solids.clear();

    PutHollowRect(Vector2(0, 0), Vector2(1, 1));
    PutSolidRect(Vector2(0, 0), Vector2(1, 1));
    UpdateScene();
}

void DebugBatch::UpdateScene()
{
    {
        CD3D11_BUFFER_DESC desc(m_Lines.size() * 4, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE);
        D3D11_SUBRESOURCE_DATA data = { m_Lines.data(), 0, 0 };
        ThrowIfFailed(m_pDevice->CreateBuffer(&desc, &data, &m_LineBuffer));
    }

    {
        CD3D11_BUFFER_DESC desc(m_Solids.size() * 4, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE);
        D3D11_SUBRESOURCE_DATA data = { m_Solids.data(), 0, 0 };
        ThrowIfFailed(m_pDevice->CreateBuffer(&desc, &data, &m_SolidBuffer));
    }
}

void DebugBatch::DrawScene(Matrix transform)
{
    if (!m_LineBuffer && !m_SolidBuffer)
        return;

    Prepare(transform, Vector4(1, 1, 1, 1));

    UINT stride = 28;
    UINT offset = 0;
    D3D11_BUFFER_DESC desc;

    m_LineBuffer->GetDesc(&desc);
    int count = desc.ByteWidth / 28;

    if (count > 8)
    {
        m_pContext->IASetVertexBuffers(0, 1, m_LineBuffer.GetAddressOf(), &stride, &offset);
        m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        m_pContext->Draw(count - 8, 8);
    }

    m_SolidBuffer->GetDesc(&desc);
    count = desc.ByteWidth / 28;

    if (count > 6)
    {
        m_pContext->IASetVertexBuffers(0, 1, m_SolidBuffer.GetAddressOf(), &stride, &offset);
        m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_pContext->Draw(count - 6, 6);
    }
}

void DebugBatch::DrawHollowSprite(Vector2 position, Vector2 size, Vector4 color, float depth, Matrix transform)
{
    Matrix scale = XMMatrixScalingFromVector(Vector3(size.x, size.y, 1));
    Matrix translation = XMMatrixTranslationFromVector(Vector3(position.x, position.y, depth));
    Prepare(scale * translation * transform, color);

    UINT stride = 28;
    UINT offset = 0;

    m_pContext->IASetVertexBuffers(0, 1, m_LineBuffer.GetAddressOf(), &stride, &offset);
    m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    m_pContext->Draw(8, 0);
}

void DebugBatch::DrawSolidSprite(Vector2 position, Vector2 size, Vector4 color, float depth, Matrix transform)
{
    Matrix scale = XMMatrixScalingFromVector(Vector3(size.x, size.y, 1));
    Matrix translation = XMMatrixTranslationFromVector(Vector3(position.x, position.y, depth));
    Prepare(scale * translation * transform, color);

    UINT stride = 28;
    UINT offset = 0;

    m_pContext->IASetVertexBuffers(0, 1, m_SolidBuffer.GetAddressOf(), &stride, &offset);
    m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pContext->Draw(6, 0);
}

void DebugBatch::Prepare(Matrix transform, Vector4 color)
{
    UINT numViewports = 1;
    D3D11_VIEWPORT viewport;
    m_pContext->RSGetViewports(&numViewports, &viewport);

    Matrix view = XMMatrixOrthographicOffCenterLH(0, viewport.Width, viewport.Height, 0, 0, 1);
    m_Constants.MVP = (transform * view).Transpose();
    m_Constants.Color = color;

    D3D11_MAPPED_SUBRESOURCE map;
    ThrowIfFailed(m_pContext->Map(m_ConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map));
    CopyMemory(map.pData, &m_Constants, sizeof(m_Constants));
    m_pContext->Unmap(m_ConstantBuffer.Get(), 0);

    m_pContext->IASetInputLayout(m_Layout.Get());
    m_pContext->VSSetShader(m_VertexShader.Get(), nullptr, 0);
    m_pContext->PSSetShader(m_PixelShader.Get(), nullptr, 0);
    m_pContext->VSSetConstantBuffers(0, 1, m_ConstantBuffer.GetAddressOf());
}
