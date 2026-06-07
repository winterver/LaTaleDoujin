#include <wincodec.h>
#include <wrl/client.h>
#include <d3d11_1.h>
#include <memory>
#include "DXHelper.h"

using Microsoft::WRL::ComPtr;

static IWICImagingFactory* GetFactory()
{
    static ComPtr<IWICImagingFactory> factory = nullptr;

    if (!factory.Get()) {
        ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                                       CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)));
    }

    return factory.Get();
}

void CreateTextureFromFile(
    ID3D11Device* pDevice,
    _In_opt_ ID3D11DeviceContext* pContext,
    const WCHAR *szFileName,
    _Outptr_ ID3D11ShaderResourceView** textureView)
{
    IWICImagingFactory *factory = GetFactory();
    ComPtr<IWICBitmapDecoder> decoder;
    ComPtr<IWICFormatConverter> converter;
    ComPtr<IWICBitmapFrameDecode> frame;

    ThrowIfFailed(factory->CreateDecoderFromFilename(
                           szFileName, nullptr, GENERIC_READ,
                           WICDecodeMetadataCacheOnDemand, &decoder));
    ThrowIfFailed(factory->CreateFormatConverter(&converter));
    ThrowIfFailed(decoder->GetFrame(0, &frame));

    converter->Initialize(frame.Get(),                  // Input bitmap to convert
                          GUID_WICPixelFormat32bppRGBA, // Destination pixel format
                          WICBitmapDitherTypeNone,      // Specified dither pattern
                          nullptr,                      // Specify a particular palette
                          0.0f,                         // Alpha threshold
                          WICBitmapPaletteTypeCustom);  // Palette translation type

    UINT width;
    UINT height;
    frame->GetSize(&width, &height);

    UINT stride = width * 4;
    UINT size = width * height * 4;

    std::unique_ptr<UINT8[]> data(new UINT8[size]);
    converter->CopyPixels(nullptr, stride, size, data.get());

    D3D11_TEXTURE2D_DESC desc = { };
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = pContext ? 0u : 1u;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;

    if (pContext)
    {
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }
    else
    {
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    }

    D3D11_SUBRESOURCE_DATA initData = { data.get(), stride, size};

    ComPtr<ID3D11Texture2D> texture;
    ThrowIfFailed(pDevice->CreateTexture2D(&desc, pContext ? nullptr : &initData, &texture));

    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = { };
    SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Texture2D.MipLevels = pContext ? unsigned(-1) : 1u;

    ThrowIfFailed(pDevice->CreateShaderResourceView(texture.Get(), &SRVDesc, textureView));

    if (pContext)
    {
        pContext->UpdateSubresource(texture.Get(), 0, nullptr, data.get(), stride, size);
        pContext->GenerateMips(*textureView);
    }
}