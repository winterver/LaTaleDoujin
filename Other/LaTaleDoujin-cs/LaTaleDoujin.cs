using Vortice.Direct3D11;
using Vortice.Direct3D;
using Vortice.Mathematics;
using D3DCompiler = Vortice.D3DCompiler.Compiler;

using MyDXLibrary;
using static MyDXLibrary.Utility;
using System;

namespace LaTaleDoujin;

public class LaTaleDoujin : GameApplication
{
    public LaTaleDoujin() : base("La Tale Doujin", 1600, 900)
    { }

    protected override void InitScene()
    {
        base.InitScene();
        ChangeScene(new TestScene(this));
    }
}

internal class TestScene : GameScene, IDisposable
{
    ID3D11ShaderResourceView _irisTexture;
    string source = """
        struct PSInput
        {
            float4 Position : SV_POSITION;
            float2 UV       : TEXCOORD0;
        };

        static const float3 vertices[6] =
        {
            float3(-1, -1, 0),
            float3(-1,  1, 0),
            float3( 1,  1, 0),
            float3(-1, -1, 0),
            float3( 1,  1, 0),
            float3( 1, -1, 0),
        };

        static const float2 texcoords[6] =
        {
            float2(0, 1),
            float2(0, 0),
            float2(1, 0),
            float2(0, 1),
            float2(1, 0),
            float2(1, 1),
        };

        PSInput VSMain(uint id : SV_VertexID)
        {
            PSInput result;

            result.Position = float4(vertices[id], 1);
            result.UV = texcoords[id];

            return result;
        }

        Texture2D tex : register(t0);
        SamplerState smp : register(s0);

        float4 PSMain(PSInput input) : SV_TARGET
        {
            return tex.Sample(smp, input.UV);
        }
    """;
    ID3D11VertexShader _vertexShader;
    ID3D11PixelShader _pixelShader;
    /*
    ID3D11InputLayout _layout;
    ID3D11SamplerState _sampler;
    ID3D11Buffer _vertexBuffer;
    */

    public TestScene(GameApplication app) : base(app)
    { }

    public void Dispose()
    {
        //_sampler?.Dispose();
        //_layout?.Dispose();
        _pixelShader?.Dispose();
        _vertexShader?.Dispose();
        _irisTexture?.Dispose();
    }

    internal override void InitScene()
    {
        _irisTexture = CreateTextureFromFile(Device, null, "../../../../Resource/IRIS.PNG");

        var vs = D3DCompiler.Compile(source, "VSMain", null, "vs_5_0");
        var ps = D3DCompiler.Compile(source, "PSMain", null, "ps_5_0");
        _vertexShader = Device.CreateVertexShader(vs.Span);
        _pixelShader = Device.CreatePixelShader(ps.Span);

        /*
        SamplerDescription samplerDesc = default;
        samplerDesc.Filter = Filter.MinMagMipLinear;
        samplerDesc.AddressU = TextureAddressMode.Wrap;
        samplerDesc.AddressV = TextureAddressMode.Wrap;
        samplerDesc.AddressW = TextureAddressMode.Wrap;
        samplerDesc.ComparisonFunc = ComparisonFunction.Never;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = float.MaxValue;
        _sampler = Device.CreateSamplerState(samplerDesc);

        InputElementDescription[] layoutDesc =
        {
            new("POSITION", 0, Format.R32G32B32_Float, 0, 0, InputClassification.PerVertexData, 0),
            new("COLOR", 0, Format.R32G32B32_Float, 0, 12, InputClassification.PerVertexData, 0),
        };

        _layout = Device.CreateInputLayout(layoutDesc, vs.Span);

        BufferDescription bufferDesc = new()
        */
    }

    internal override void DrawScene()
    {
        Context.ClearRenderTargetView(RenderTargetView, Colors.CornflowerBlue);
        Context.ClearDepthStencilView(DepthStencilView, DepthStencilClearFlags.Depth | DepthStencilClearFlags.Stencil, 1.0f, 0);

        Context.IASetPrimitiveTopology(PrimitiveTopology.TriangleList);
        Context.VSSetShader(_vertexShader);
        Context.PSSetShader(_pixelShader);
        Context.PSSetShaderResource(0, _irisTexture);
        Context.Draw(6, 0);

        SwapChain.Present(0, 0);
    }
}