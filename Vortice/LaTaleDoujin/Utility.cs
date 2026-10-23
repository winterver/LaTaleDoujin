using System;
using Vortice.Direct3D11;
using Vortice.Direct3D;
using Vortice.DXGI;
using Vortice.WIC;

namespace MyDXLibrary;

public static class Utility
{
    private static readonly Lazy<IWICImagingFactory> s_factory = new(() => new IWICImagingFactory());
    private static IWICImagingFactory _factory => s_factory.Value;

    public static ID3D11ShaderResourceView CreateTextureFromFile(ID3D11Device device, ID3D11DeviceContext context, string path)
    {
        try
        {
            return _createTextureFromFile(device, context, path);
        }
        catch(Exception e)
        {
            Console.Error.Write(e.StackTrace);
        }

        return null;
    }

    private static unsafe ID3D11ShaderResourceView _createTextureFromFile(ID3D11Device device, ID3D11DeviceContext context, string path)
    {
        using IWICBitmapDecoder decoder = _factory.CreateDecoderFromFileName(path);
        using IWICFormatConverter converter = _factory.CreateFormatConverter();
        using IWICBitmapFrameDecode frame = decoder.GetFrame(0);

        converter.Initialize(frame, PixelFormat.Format32bppRGBA,
            BitmapDitherType.None, default, 0, BitmapPaletteType.Custom);

        uint width;
        uint height;
        frame.GetSize(out width, out height);

        byte[] data = new byte[width * height * 4];
        converter.CopyPixels(width * 4, data);

        Texture2DDescription desc = default;
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = context != null ? 0u : 1u;
        desc.ArraySize = 1;
        desc.Format = Format.R8G8B8A8_UNorm;
        desc.SampleDescription.Count = 1;
        desc.SampleDescription.Quality = 0;
        desc.Usage = ResourceUsage.Default;

        if (context != null)
        {
            desc.BindFlags = BindFlags.ShaderResource | BindFlags.RenderTarget;
            desc.MiscFlags = ResourceOptionFlags.GenerateMips;
        }
        else
        {
            desc.BindFlags = BindFlags.ShaderResource;
        }

        ID3D11Texture2D texture;

        if (context != null)
        {
            texture = device.CreateTexture2D(desc);
        }
        else fixed(byte* ptr = data)
        {
            SubresourceData initData = new(ptr, width * 4, (uint)data.Length);
            texture = device.CreateTexture2D(desc, initData);
        }

        ShaderResourceViewDescription viewDesc = default;
        viewDesc.Format = Format.R8G8B8A8_UNorm;
        viewDesc.ViewDimension = ShaderResourceViewDimension.Texture2D;
        viewDesc.Texture2D.MipLevels = context != null ? uint.MaxValue : 1u;

        ID3D11ShaderResourceView view = device.CreateShaderResourceView(texture, viewDesc);
        context?.UpdateSubresource(data, texture, 0, width * 4);
        context?.GenerateMips(view);

        return view;
    }
}
