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

    CD3D11_BUFFER_DESC bufferDesc(sizeof(mConstants), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

    mDevice = device;
    mDevice->GetImmediateContext(&mContext);

    ThrowIfFailed(D3DCompile(source, sizeof(source), nullptr, nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexBlob, nullptr));
    ThrowIfFailed(D3DCompile(source, sizeof(source), nullptr, nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelBlob, nullptr));
    ThrowIfFailed(device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, &mVertexShader));
    ThrowIfFailed(device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, &mPixelShader));
    ThrowIfFailed(device->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &mLayout));
    ThrowIfFailed(device->CreateBuffer(&bufferDesc, nullptr, &mConstantBuffer));

    PutHollowRect(Vector2(0, 0), Vector2(1, 1));
    PutSolidRect(Vector2(0, 0), Vector2(1, 1));
    UpdateScene();
}

void DebugBatch::PutLine(Vector2 point1, Vector2 point2, Vector4 color, float depth)
{
    mLines.push_back(point1.x); mLines.push_back(point1.y); mLines.push_back(depth);
    mLines.push_back(color.x); mLines.push_back(color.y); mLines.push_back(color.z); mLines.push_back(color.w);
    mLines.push_back(point2.x); mLines.push_back(point2.y); mLines.push_back(depth);
    mLines.push_back(color.x); mLines.push_back(color.y); mLines.push_back(color.z); mLines.push_back(color.w);
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
    mSolids.push_back(position.x); mSolids.push_back(position.y); mSolids.push_back(depth);
    mSolids.push_back(color.x); mSolids.push_back(color.y); mSolids.push_back(color.z); mSolids.push_back(color.w);
    mSolids.push_back(position.x+size.x); mSolids.push_back(position.y); mSolids.push_back(depth);
    mSolids.push_back(color.x); mSolids.push_back(color.y); mSolids.push_back(color.z); mSolids.push_back(color.w);
    mSolids.push_back(position.x+size.x); mSolids.push_back(position.y+size.y); mSolids.push_back(depth);
    mSolids.push_back(color.x); mSolids.push_back(color.y); mSolids.push_back(color.z); mSolids.push_back(color.w);

    mSolids.push_back(position.x+size.x); mSolids.push_back(position.y+size.y); mSolids.push_back(depth);
    mSolids.push_back(color.x); mSolids.push_back(color.y); mSolids.push_back(color.z); mSolids.push_back(color.w);
    mSolids.push_back(position.x); mSolids.push_back(position.y+size.y); mSolids.push_back(depth);
    mSolids.push_back(color.x); mSolids.push_back(color.y); mSolids.push_back(color.z); mSolids.push_back(color.w);
    mSolids.push_back(position.x); mSolids.push_back(position.y); mSolids.push_back(depth);
    mSolids.push_back(color.x); mSolids.push_back(color.y); mSolids.push_back(color.z); mSolids.push_back(color.w);
}

void DebugBatch::ClearScene()
{
    mLineBuffer.Reset();
    mSolidBuffer.Reset();
    mLines.clear();
    mSolids.clear();

    PutHollowRect(Vector2(0, 0), Vector2(1, 1));
    PutSolidRect(Vector2(0, 0), Vector2(1, 1));
    UpdateScene();
}

void DebugBatch::UpdateScene()
{
    {
        CD3D11_BUFFER_DESC desc(mLines.size() * 4, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE);
        D3D11_SUBRESOURCE_DATA data = { mLines.data(), 0, 0 };
        ThrowIfFailed(mDevice->CreateBuffer(&desc, &data, &mLineBuffer));
    }

    {
        CD3D11_BUFFER_DESC desc(mSolids.size() * 4, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE);
        D3D11_SUBRESOURCE_DATA data = { mSolids.data(), 0, 0 };
        ThrowIfFailed(mDevice->CreateBuffer(&desc, &data, &mSolidBuffer));
    }
}

void DebugBatch::DrawScene(Matrix transform)
{
    if (!mLineBuffer && !mSolidBuffer)
        return;

    Prepare(transform, Vector4(1, 1, 1, 1));

    UINT stride = 28;
    UINT offset = 0;
    D3D11_BUFFER_DESC desc;

    mLineBuffer->GetDesc(&desc);
    int count = desc.ByteWidth / 28;

    if (count > 8)
    {
        mContext->IASetVertexBuffers(0, 1, mLineBuffer.GetAddressOf(), &stride, &offset);
        mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        mContext->Draw(count - 8, 8);
    }

    mSolidBuffer->GetDesc(&desc);
    count = desc.ByteWidth / 28;

    if (count > 6)
    {
        mContext->IASetVertexBuffers(0, 1, mSolidBuffer.GetAddressOf(), &stride, &offset);
        mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        mContext->Draw(count - 6, 6);
    }
}

void DebugBatch::DrawHollowSprite(Vector2 position, Vector2 size, Vector4 color, float depth, Matrix transform)
{
    Matrix scale = XMMatrixScalingFromVector(Vector3(size.x, size.y, 1));
    Matrix translation = XMMatrixTranslationFromVector(Vector3(position.x, position.y, depth));
    Prepare(scale * translation * transform, color);

    UINT stride = 28;
    UINT offset = 0;

    mContext->IASetVertexBuffers(0, 1, mLineBuffer.GetAddressOf(), &stride, &offset);
    mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    mContext->Draw(8, 0);
}

void DebugBatch::DrawSolidSprite(Vector2 position, Vector2 size, Vector4 color, float depth, Matrix transform)
{
    Matrix scale = XMMatrixScalingFromVector(Vector3(size.x, size.y, 1));
    Matrix translation = XMMatrixTranslationFromVector(Vector3(position.x, position.y, depth));
    Prepare(scale * translation * transform, color);

    UINT stride = 28;
    UINT offset = 0;

    mContext->IASetVertexBuffers(0, 1, mSolidBuffer.GetAddressOf(), &stride, &offset);
    mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mContext->Draw(6, 0);
}

void DebugBatch::Prepare(Matrix transform, Vector4 color)
{
    UINT numViewports = 1;
    D3D11_VIEWPORT viewport;
    mContext->RSGetViewports(&numViewports, &viewport);

    Matrix view = XMMatrixOrthographicOffCenterLH(0, viewport.Width, viewport.Height, 0, 0, 1);
    mConstants.MVP = (transform * view).Transpose();
    mConstants.Color = color;

    D3D11_MAPPED_SUBRESOURCE map;
    ThrowIfFailed(mContext->Map(mConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map));
    CopyMemory(map.pData, &mConstants, sizeof(mConstants));
    mContext->Unmap(mConstantBuffer.Get(), 0);

    mContext->IASetInputLayout(mLayout.Get());
    mContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
    mContext->PSSetShader(mPixelShader.Get(), nullptr, 0);
    mContext->VSSetConstantBuffers(0, 1, mConstantBuffer.GetAddressOf());
}
