#pragma once
#include <Windows.h>
#include <d3d11_1.h>

void CreateTextureFromFile(
    ID3D11Device* pDevice,
    _In_opt_ ID3D11DeviceContext* pContext,
    const WCHAR* szFileName,
    _Outptr_ ID3D11ShaderResourceView** textureView);
