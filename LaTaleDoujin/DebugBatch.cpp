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
            float4 position : SV_POSITION;
            float4 color : COLOR;
        };

        matrix MVP : register(b0);

        PSInput VSMain(float3 position : POSITION, float4 color : COLOR)
        {
            PSInput result;

            result.position = mul(float4(position, 1), MVP);
            result.color = color;

            return result;
        }

        float4 PSMain(PSInput input) : SV_TARGET
        {
            return input.color;
        }
    )";

    const D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    CD3D11_BUFFER_DESC bufferDesc(64, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    ThrowIfFailed(D3DCompile(source, sizeof(source), nullptr, nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexBlob, nullptr));
    ThrowIfFailed(D3DCompile(source, sizeof(source), nullptr, nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelBlob, nullptr));
    ThrowIfFailed(device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, &m_VertexShader));
    ThrowIfFailed(device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, &m_PixelShader));
    ThrowIfFailed(device->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &m_Layout));

    m_pDevice = device;
    m_pDevice->GetImmediateContext(&m_pContext);
    ThrowIfFailed(m_pDevice->CreateBuffer(&bufferDesc, nullptr, &m_MVPBuffer));
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
    m_Lines.clear();
    m_Solids.clear();
    m_LineBuffer.Reset();
    m_SolidBuffer.Reset();
}

void DebugBatch::UpdateScene()
{
    if (m_Lines.size())
    {
        CD3D11_BUFFER_DESC desc(m_Lines.size() * 4, D3D11_BIND_VERTEX_BUFFER);
        D3D11_SUBRESOURCE_DATA data = { m_Lines.data(), 0, 0 };
        ThrowIfFailed(m_pDevice->CreateBuffer(&desc, &data, &m_LineBuffer));
    }

    if (m_Solids.size())
    {
        CD3D11_BUFFER_DESC desc(m_Solids.size() * 4, D3D11_BIND_VERTEX_BUFFER);
        D3D11_SUBRESOURCE_DATA data = { m_Solids.data(), 0, 0 };
        ThrowIfFailed(m_pDevice->CreateBuffer(&desc, &data, &m_SolidBuffer));
    }
}

void DebugBatch::DrawScene(Matrix transform)
{
    if (!m_LineBuffer && !m_SolidBuffer)
        return;

    UINT numViewports = 1;
    D3D11_VIEWPORT viewport;
    m_pContext->RSGetViewports(&numViewports, &viewport);

    Matrix view = XMMatrixOrthographicOffCenterLH(0, viewport.Width, viewport.Height, 0, 0, 1);
    Matrix MVP = (transform * view).Transpose();

    D3D11_MAPPED_SUBRESOURCE map;
    ThrowIfFailed(m_pContext->Map(m_MVPBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map));
    CopyMemory(map.pData, &MVP, sizeof(MVP));
    m_pContext->Unmap(m_MVPBuffer.Get(), 0);

    m_pContext->IASetInputLayout(m_Layout.Get());
    m_pContext->VSSetShader(m_VertexShader.Get(), nullptr, 0);
    m_pContext->PSSetShader(m_PixelShader.Get(), nullptr, 0);
    m_pContext->VSSetConstantBuffers(0, 1, m_MVPBuffer.GetAddressOf());

    UINT stride = 28;
    UINT offset = 0;

    if (m_LineBuffer)
    {
        D3D11_BUFFER_DESC desc;
        m_LineBuffer->GetDesc(&desc);
        m_pContext->IASetVertexBuffers(0, 1, m_LineBuffer.GetAddressOf(), &stride, &offset);
        m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        m_pContext->Draw(desc.ByteWidth / 28, 0);
    }

    if (m_SolidBuffer)
    {
        D3D11_BUFFER_DESC desc;
        m_SolidBuffer->GetDesc(&desc);
        m_pContext->IASetVertexBuffers(0, 1, m_SolidBuffer.GetAddressOf(), &stride, &offset);
        m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_pContext->Draw(desc.ByteWidth / 28, 0);
    }
}
