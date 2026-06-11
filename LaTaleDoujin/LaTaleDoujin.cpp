#include "LaTaleDoujin.h"
#include "Utility.h"
#include <SimpleMath.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

struct VS_BLOOM_PARAMETERS
{
    float bloomThreshold;
    float blurAmount;
    float bloomIntensity;
    float baseIntensity;
    float bloomSaturation;
    float baseSaturation;
    uint8_t na[8];
};

static const VS_BLOOM_PARAMETERS bloomPresets[] =
{
    //Thresh  Blur Bloom  Base  BloomSat BaseSat
    { 0.25f,  4,   1.25f, 1,    1,       1 }, // Default
    { 0,      3,   1,     1,    1,       1 }, // Soft
    { 0.5f,   8,   2,     1,    0,       1 }, // Desaturated
    { 0.25f,  4,   2,     1,    2,       0 }, // Saturated
    { 0,      2,   1,     0.1f, 1,       1 }, // Blurry
    { 0.5f,   2,   1,     1,    1,       1 }, // Subtle
    { 0.25f,  4,   1.25f, 1,    1,       1 }, // None
};

struct VS_BLUR_PARAMETERS
{
    static constexpr size_t SAMPLE_COUNT = 15;

    XMFLOAT4 sampleOffsets[SAMPLE_COUNT];
    XMFLOAT4 sampleWeights[SAMPLE_COUNT];

    void SetBlurEffectParameters(float dx, float dy, float blurAmount)
    {
        sampleWeights[0].x = ComputeGaussian(0, blurAmount);
        sampleOffsets[0].x = sampleOffsets[0].y = 0.f;

        float totalWeights = sampleWeights[0].x;

        // Add pairs of additional sample taps, positioned
        // along a line in both directions from the center.
        for (size_t i = 0; i < SAMPLE_COUNT / 2; i++)
        {
            // Store weights for the positive and negative taps.
            float weight = ComputeGaussian(float(i + 1.f), blurAmount);

            sampleWeights[i * 2 + 1].x = weight;
            sampleWeights[i * 2 + 2].x = weight;

            totalWeights += weight * 2;

            // To get the maximum amount of blurring from a limited number of
            // pixel shader samples, we take advantage of the bilinear filtering
            // hardware inside the texture fetch unit. If we position our texture
            // coordinates exactly halfway between two texels, the filtering unit
            // will average them for us, giving two samples for the price of one.
            // This allows us to step in units of two texels per sample, rather
            // than just one at a time. The 1.5 offset kicks things off by
            // positioning us nicely in between two texels.
            float sampleOffset = float(i) * 2.f + 1.5f;

            Vector2 delta = Vector2(dx, dy) * sampleOffset;

            // Store texture coordinate offsets for the positive and negative taps.
            sampleOffsets[i * 2 + 1].x = delta.x;
            sampleOffsets[i * 2 + 1].y = delta.y;
            sampleOffsets[i * 2 + 2].x = -delta.x;
            sampleOffsets[i * 2 + 2].y = -delta.y;
        }

        for (size_t i = 0; i < SAMPLE_COUNT; i++)
        {
            sampleWeights[i].x /= totalWeights;
        }
    }

private:
    float ComputeGaussian(float n, float theta)
    {
        return (float)((1.0 / sqrtf(2 * XM_PI * theta))
            * expf(-(n * n) / (2 * theta * theta)));
    }
};

LaTaleDoujin::LaTaleDoujin()
    : D3D11Application(L"La Tale Doujin", 1360, 768)
{
}

LaTaleDoujin::~LaTaleDoujin() = default;

bool LaTaleDoujin::Init()
{
    bool val = D3D11Application::Init();
    if (!val) return false;

    m_SpriteBatch = std::make_unique<SpriteBatch>(m_pContext.Get());

    LPVOID bytecode;
    SIZE_T length;

    LoadDataFromResource(bytecode, length, L"GaussianBlur", L"HLSL");
    ThrowIfFailed(m_pDevice->CreatePixelShader(bytecode, length, nullptr, &m_GaussianBlur)); 

    {
        VS_BLUR_PARAMETERS blurDataH = { };
        VS_BLUR_PARAMETERS blurDataV = { };
        blurDataH.SetBlurEffectParameters(1.f / (float(800) / 2), 0, 2);
        blurDataV.SetBlurEffectParameters(0, 1.f / (float(800) / 2), 2);

        CD3D11_BUFFER_DESC desc(sizeof(VS_BLUR_PARAMETERS), D3D11_BIND_CONSTANT_BUFFER);
        D3D11_SUBRESOURCE_DATA initData = { };

        initData.pSysMem = &blurDataH;
        ThrowIfFailed(m_pDevice->CreateBuffer(&desc, &initData, &m_GaussianParametersH));
        initData.pSysMem = &blurDataV;
        ThrowIfFailed(m_pDevice->CreateBuffer(&desc, &initData, &m_GaussianParametersV));
    }

    ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    CreateTextureFromFile(m_pDevice.Get(), nullptr, L"C:/Data/Develop/LaTaleDoujin_Sharp/LaTaleDoujin/resources/IRIS.PNG", &m_IrisTexture);

    return true;
}

void LaTaleDoujin::OnResize()
{
    D3D11Application::OnResize();

    m_Framebuffer1.Reset();
    m_Framebuffer2.Reset();
    m_Framebuffer1View.Reset();
    m_Framebuffer2View.Reset();
    m_Framebuffer1Tex.Reset();
    m_Framebuffer2Tex.Reset();

    CD3D11_TEXTURE2D_DESC desc1(DXGI_FORMAT_R8G8B8A8_UNORM, m_Width, m_Height, 1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
    CD3D11_RENDER_TARGET_VIEW_DESC desc2(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM);
    CD3D11_SHADER_RESOURCE_VIEW_DESC desc3(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM);

    ThrowIfFailed(m_pDevice->CreateTexture2D(&desc1, nullptr, &m_Framebuffer1));
    ThrowIfFailed(m_pDevice->CreateTexture2D(&desc1, nullptr, &m_Framebuffer2));
    ThrowIfFailed(m_pDevice->CreateRenderTargetView(m_Framebuffer1.Get(), &desc2, &m_Framebuffer1View));
    ThrowIfFailed(m_pDevice->CreateRenderTargetView(m_Framebuffer2.Get(), &desc2, &m_Framebuffer2View));
    ThrowIfFailed(m_pDevice->CreateShaderResourceView(m_Framebuffer1.Get(), &desc3, &m_Framebuffer1Tex));
    ThrowIfFailed(m_pDevice->CreateShaderResourceView(m_Framebuffer2.Get(), &desc3, &m_Framebuffer2Tex));
}

void LaTaleDoujin::UpdateScene()
{
}

void LaTaleDoujin::DrawScene()
{
    static float cornflowerblue[] = { 100.0f/255, 149.0f/255, 237.0f/255, 1.0f };
    m_pContext->ClearRenderTargetView(m_pRenderTargetView.Get(), cornflowerblue);
    m_pContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    ID3D11ShaderResourceView* texView = nullptr;
    m_pContext->PSSetShaderResources(0, 1, &texView);

    m_pContext->OMSetRenderTargets(1, m_Framebuffer1View.GetAddressOf(), m_pDepthStencilView.Get());
    m_SpriteBatch->Begin(SpriteSortMode_Deferred, nullptr, nullptr, nullptr, nullptr, [=]
        {
            m_pContext->PSSetShader(m_GaussianBlur.Get(), nullptr, 0);
            m_pContext->PSSetConstantBuffers(0, 1, m_GaussianParametersH.GetAddressOf());
        });
    m_SpriteBatch->Draw(m_IrisTexture.Get(), XMFLOAT2());
    m_SpriteBatch->End();

    m_pContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    m_pContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());
    m_SpriteBatch->Begin(SpriteSortMode_Deferred, nullptr, nullptr, nullptr, nullptr, [=]
        {
            m_pContext->PSSetShader(m_GaussianBlur.Get(), nullptr, 0);
            m_pContext->PSSetConstantBuffers(0, 1, m_GaussianParametersV.GetAddressOf());
        });
    m_SpriteBatch->Draw(m_Framebuffer1Tex.Get(), XMFLOAT2());
    m_SpriteBatch->End();

    m_pSwapChain->Present(0, 0);
}

