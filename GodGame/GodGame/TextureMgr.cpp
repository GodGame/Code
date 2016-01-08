#include "stdafx.h"
#include "MyInline.h"
#include "TextureMgr.h"


CTexture::CTexture(int nTextures, int nSamplers, int nTextureStartSlot, int nSamplerStartSlot, SETSHADER nSetInfo)
{
	m_nReferences = 0;
	m_uTextureSet = nSetInfo;

	m_nTextures = nTextures;
	m_ppd3dsrvTextures = new ID3D11ShaderResourceView*[m_nTextures];

	for (int i = 0; i < m_nTextures; i++) m_ppd3dsrvTextures[i] = nullptr;
	m_nTextureStartSlot = nTextureStartSlot;
	m_nSamplers = nSamplers;
	m_ppd3dSamplerStates = new ID3D11SamplerState*[m_nSamplers];

	for (int i = 0; i < m_nSamplers; i++) m_ppd3dSamplerStates[i] = nullptr;
	m_nSamplerStartSlot = nSamplerStartSlot;
}

CTexture::~CTexture()
{
	for (int i = 0; i < m_nTextures; i++) if (m_ppd3dsrvTextures[i]) m_ppd3dsrvTextures[i]->Release();
	for (int i = 0; i < m_nSamplers; i++) if (m_ppd3dSamplerStates[i]) m_ppd3dSamplerStates[i]->Release();
	if (m_ppd3dsrvTextures) delete[] m_ppd3dsrvTextures;
	if (m_ppd3dSamplerStates) delete[] m_ppd3dSamplerStates;
}

void CTexture::SetTexture(int nIndex, ID3D11ShaderResourceView *pd3dsrvTexture)
{
	if (m_ppd3dsrvTextures[nIndex]) m_ppd3dsrvTextures[nIndex]->Release();
	m_ppd3dsrvTextures[nIndex] = pd3dsrvTexture;
	if (pd3dsrvTexture) pd3dsrvTexture->AddRef();
}

void CTexture::SetSampler(int nIndex, ID3D11SamplerState *pd3dSamplerState)
{
	if (m_ppd3dSamplerStates[nIndex]) m_ppd3dSamplerStates[nIndex]->Release();
	m_ppd3dSamplerStates[nIndex] = pd3dSamplerState;
	if (pd3dSamplerState) pd3dSamplerState->AddRef();
}

void CTexture::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext)
{
	if (m_uTextureSet & SET_SHADER_VS)
	{
		pd3dDeviceContext->VSSetShaderResources(m_nTextureStartSlot, m_nTextures, m_ppd3dsrvTextures);
		pd3dDeviceContext->VSSetSamplers(m_nSamplerStartSlot, m_nSamplers, m_ppd3dSamplerStates);
	}
	if (m_uTextureSet & SET_SHADER_HS)
	{
		pd3dDeviceContext->HSSetShaderResources(m_nTextureStartSlot, m_nTextures, m_ppd3dsrvTextures);
		pd3dDeviceContext->HSSetSamplers(m_nSamplerStartSlot, m_nSamplers, m_ppd3dSamplerStates);
	}
	if (m_uTextureSet & SET_SHADER_DS)
	{
		pd3dDeviceContext->DSSetShaderResources(m_nTextureStartSlot, m_nTextures, m_ppd3dsrvTextures);
		pd3dDeviceContext->DSSetSamplers(m_nSamplerStartSlot, m_nSamplers, m_ppd3dSamplerStates);
	}
	if (m_uTextureSet & SET_SHADER_PS)
	{
		pd3dDeviceContext->PSSetShaderResources(m_nTextureStartSlot, m_nTextures, m_ppd3dsrvTextures);
		pd3dDeviceContext->PSSetSamplers(m_nSamplerStartSlot, m_nSamplers, m_ppd3dSamplerStates);
	}
	if (m_uTextureSet & SET_SHADER_GS)
	{
		pd3dDeviceContext->GSSetShaderResources(m_nTextureStartSlot, m_nTextures, m_ppd3dsrvTextures);
		pd3dDeviceContext->GSSetSamplers(m_nSamplerStartSlot, m_nSamplers, m_ppd3dSamplerStates);
	}
	if (m_uTextureSet & SET_SHADER_CS)
	{
		pd3dDeviceContext->CSSetShaderResources(m_nTextureStartSlot, m_nTextures, m_ppd3dsrvTextures);
		pd3dDeviceContext->CSSetSamplers(m_nSamplerStartSlot, m_nSamplers, m_ppd3dSamplerStates);
	}
}

void CTexture::UpdateTextureShaderVariable(ID3D11DeviceContext *pd3dDeviceContext)
{
	if (m_uTextureSet & SET_SHADER_VS) pd3dDeviceContext->VSSetShaderResources(m_nTextureStartSlot, m_nTextures, m_ppd3dsrvTextures);
	if (m_uTextureSet & SET_SHADER_PS) pd3dDeviceContext->PSSetShaderResources(m_nTextureStartSlot, m_nTextures, m_ppd3dsrvTextures);
	if (m_uTextureSet & SET_SHADER_HS) pd3dDeviceContext->HSSetShaderResources(m_nTextureStartSlot, m_nTextures, m_ppd3dsrvTextures);
	if (m_uTextureSet & SET_SHADER_DS) pd3dDeviceContext->DSSetShaderResources(m_nTextureStartSlot, m_nTextures, m_ppd3dsrvTextures);
	if (m_uTextureSet & SET_SHADER_GS) pd3dDeviceContext->GSSetShaderResources(m_nTextureStartSlot, m_nTextures, m_ppd3dsrvTextures);
	if (m_uTextureSet & SET_SHADER_CS) pd3dDeviceContext->CSSetShaderResources(m_nTextureStartSlot, m_nTextures, m_ppd3dsrvTextures);
}

void CTexture::UpdateSamplerShaderVariable(ID3D11DeviceContext *pd3dDeviceContext)
{
	if (m_uTextureSet & SET_SHADER_VS) pd3dDeviceContext->VSSetSamplers(m_nSamplerStartSlot, m_nSamplers, m_ppd3dSamplerStates);
	if (m_uTextureSet & SET_SHADER_PS) pd3dDeviceContext->PSSetSamplers(m_nSamplerStartSlot, m_nSamplers, m_ppd3dSamplerStates);
	if (m_uTextureSet & SET_SHADER_HS) pd3dDeviceContext->HSSetSamplers(m_nSamplerStartSlot, m_nSamplers, m_ppd3dSamplerStates);
	if (m_uTextureSet & SET_SHADER_DS) pd3dDeviceContext->DSSetSamplers(m_nSamplerStartSlot, m_nSamplers, m_ppd3dSamplerStates);
	if (m_uTextureSet & SET_SHADER_GS) pd3dDeviceContext->GSSetSamplers(m_nSamplerStartSlot, m_nSamplers, m_ppd3dSamplerStates);
	if (m_uTextureSet & SET_SHADER_CS) pd3dDeviceContext->CSSetSamplers(m_nSamplerStartSlot, m_nSamplers, m_ppd3dSamplerStates);
}

ID3D11ShaderResourceView *CTexture::CreateTexture2DArraySRV(ID3D11Device *pd3dDevice, wchar_t *ppstrFilePaths, UINT nTextures)
{
	D3DX11_IMAGE_LOAD_INFO d3dxImageLoadInfo;
	d3dxImageLoadInfo.Width = D3DX11_FROM_FILE;
	d3dxImageLoadInfo.Height = D3DX11_FROM_FILE;
	d3dxImageLoadInfo.Depth = D3DX11_FROM_FILE;
	d3dxImageLoadInfo.FirstMipLevel = 0;
	d3dxImageLoadInfo.MipLevels = D3DX11_FROM_FILE;
	d3dxImageLoadInfo.Usage = D3D11_USAGE_STAGING;
	d3dxImageLoadInfo.BindFlags = 0;
	d3dxImageLoadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	d3dxImageLoadInfo.MiscFlags = 0;
	d3dxImageLoadInfo.Format = DXGI_FORMAT_FROM_FILE; //DXGI_FORMAT_R8G8B8A8_UNORM
	d3dxImageLoadInfo.Filter = D3DX11_FILTER_NONE;
	d3dxImageLoadInfo.MipFilter = D3DX11_FILTER_LINEAR;
	d3dxImageLoadInfo.pSrcInfo = 0;

	ID3D11Texture2D **ppd3dTextures = new ID3D11Texture2D*[nTextures];
	_TCHAR pstrTextureName[80];
	for (UINT i = 0; i < nTextures; i++) {
		_stprintf_s(pstrTextureName, 80, _T("%s%02d.png"), ppstrFilePaths, i + 1);
		D3DX11CreateTextureFromFile(pd3dDevice, pstrTextureName, &d3dxImageLoadInfo, 0, (ID3D11Resource **)&ppd3dTextures[i], 0);
	}
	D3D11_TEXTURE2D_DESC d3dTexure2DDesc;
	ppd3dTextures[0]->GetDesc(&d3dTexure2DDesc);

	D3D11_TEXTURE2D_DESC d3dTexture2DArrayDesc;
	d3dTexture2DArrayDesc.Width = d3dTexure2DDesc.Width;
	d3dTexture2DArrayDesc.Height = d3dTexure2DDesc.Height;
	d3dTexture2DArrayDesc.MipLevels = d3dTexure2DDesc.MipLevels;
	d3dTexture2DArrayDesc.ArraySize = nTextures;
	d3dTexture2DArrayDesc.Format = d3dTexure2DDesc.Format;
	d3dTexture2DArrayDesc.SampleDesc.Count = 1;
	d3dTexture2DArrayDesc.SampleDesc.Quality = 0;
	d3dTexture2DArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dTexture2DArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	d3dTexture2DArrayDesc.CPUAccessFlags = 0;
	d3dTexture2DArrayDesc.MiscFlags = 0;

	ID3D11Texture2D *pd3dTexture2DArray;
	pd3dDevice->CreateTexture2D(&d3dTexture2DArrayDesc, 0, &pd3dTexture2DArray);

	ID3D11DeviceContext *pd3dDeviceContext;
	pd3dDevice->GetImmediateContext(&pd3dDeviceContext);

	D3D11_MAPPED_SUBRESOURCE d3dMappedTexture2D;
	for (UINT t = 0; t < nTextures; t++)
	{
		for (UINT m = 0; m < d3dTexure2DDesc.MipLevels; m++)
		{
			pd3dDeviceContext->Map(ppd3dTextures[t], m, D3D11_MAP_READ, 0, &d3dMappedTexture2D);
			pd3dDeviceContext->UpdateSubresource(pd3dTexture2DArray, D3D11CalcSubresource(m, t, d3dTexure2DDesc.MipLevels), 0, d3dMappedTexture2D.pData, d3dMappedTexture2D.RowPitch, d3dMappedTexture2D.DepthPitch);
			pd3dDeviceContext->Unmap(ppd3dTextures[t], m);
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC d3dTextureSRVDesc;
	ZeroMemory(&d3dTextureSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	d3dTextureSRVDesc.Format = d3dTexture2DArrayDesc.Format;
	d3dTextureSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	d3dTextureSRVDesc.Texture2DArray.MostDetailedMip = 0;
	d3dTextureSRVDesc.Texture2DArray.MipLevels = d3dTexture2DArrayDesc.MipLevels;
	d3dTextureSRVDesc.Texture2DArray.FirstArraySlice = 0;
	d3dTextureSRVDesc.Texture2DArray.ArraySize = nTextures;

	ID3D11ShaderResourceView *pd3dsrvTextureArray;
	pd3dDevice->CreateShaderResourceView(pd3dTexture2DArray, &d3dTextureSRVDesc, &pd3dsrvTextureArray);

	if (pd3dTexture2DArray) pd3dTexture2DArray->Release();

	for (UINT i = 0; i < nTextures; i++) if (ppd3dTextures[i]) ppd3dTextures[i]->Release();
	delete[] ppd3dTextures;

	if (pd3dDeviceContext) pd3dDeviceContext->Release();

	return(pd3dsrvTextureArray);
}





CTextureMgr::CTextureMgr() : CMgr<CTexture>()
{
}

CTextureMgr::~CTextureMgr()
{
}

CTextureMgr & CTextureMgr::GetInstance()
{
	static CTextureMgr instance;
	return instance;
	// TODO: 여기에 반환 구문을 삽입합니다.
}

void CTextureMgr::BuildResources(ID3D11Device * pd3dDevice)
{
	BuildSamplers(pd3dDevice);
	BuildTextures(pd3dDevice);

	ID3D11ShaderResourceView *pd3dsrvTexture = nullptr;
	pd3dsrvTexture = CTextureMgr::CreateRandomTexture1DSRV(pd3dDevice);
	InsertShaderResourceView(pd3dsrvTexture, "srv_random1d", TX_SLOT_RANDOM1D, SET_SHADER_GS | SET_SHADER_PS);
	pd3dsrvTexture->Release();
}

void CTextureMgr::BuildSamplers(ID3D11Device * pd3dDevice)
{
	HRESULT hr;
	ID3D11SamplerState *pd3dSamplerState = nullptr;
	D3D11_SAMPLER_DESC d3dSamplerDesc;
	ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	{
		d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		d3dSamplerDesc.MinLOD = 0;
		d3dSamplerDesc.MaxLOD = 0;

		hr = pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState);
		assert(SUCCEEDED(hr));
		InsertSamplerState(pd3dSamplerState, "ss_linear_wrap", 0);
		pd3dSamplerState->Release();
	}
	{
		d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

		hr = pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState);
		assert(SUCCEEDED(hr));
		InsertSamplerState(pd3dSamplerState, "ss_linear_clamp", 0);
		pd3dSamplerState->Release();
	}
	{
		ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
		d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		d3dSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		d3dSamplerDesc.MaxAnisotropy = 1;
		d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;//D3D11_COMPARISON_NEVER;

		hr = pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState);
		assert(SUCCEEDED(hr));
		InsertSamplerState(pd3dSamplerState, "scs_point_border", 0);
		pd3dSamplerState->Release();
	}
	{
		d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		d3dSamplerDesc.MaxAnisotropy = 1;

		hr = pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState);
		assert(SUCCEEDED(hr));
		InsertSamplerState(pd3dSamplerState, "ss_point_border", 0);
		pd3dSamplerState->Release();
	}
}

void CTextureMgr::BuildTextures(ID3D11Device * pd3dDevice)
{
	ID3D11ShaderResourceView *pd3dsrvTexture = nullptr;
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Miscellaneous/Brick02.jpg"), nullptr, nullptr, &pd3dsrvTexture, nullptr);
	assert(SUCCEEDED(hr));

	InsertShaderResourceView(pd3dsrvTexture, "srv_brick2_jpg", 0);
	pd3dsrvTexture->Release();


}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CTextureMgr::UpdateShaderVariable(ID3D11DeviceContext * pd3dDeviceContext, string name)
{
	CTexture * pTexture = m_mpList[name];
	if (pTexture)
	{
		if(pTexture->IsSampler() && pTexture->IsSRV()) pTexture->UpdateShaderVariable(pd3dDeviceContext);
		else if (pTexture->IsSRV()) pTexture->UpdateTextureShaderVariable(pd3dDeviceContext);
		else if (pTexture->IsSampler()) pTexture->UpdateSamplerShaderVariable(pd3dDeviceContext);
	}
}

bool CTextureMgr::InsertShaderResourceView(ID3D11ShaderResourceView * pSRV, string name, UINT uSlotNum, SETSHADER nSetInfo)
{
	if (nullptr == pSRV) return false;

	CTexture * pTexture = new CTexture(1, 0, uSlotNum, 0, nSetInfo);
	pTexture->SetTexture(0, pSRV);
	m_mpList[name] = pTexture;
	return true;
}

bool CTextureMgr::InsertSamplerState(ID3D11SamplerState * pSamplerState, string name, UINT uSlotNum, SETSHADER nSetInfo)
{
	if (nullptr == pSamplerState) return false;

	CTexture * pTexture = new CTexture(0, 1, 0, uSlotNum, nSetInfo);
	pTexture->SetSampler(0, pSamplerState);
	m_mpList[name] = pTexture;
	return true;
}

ID3D11ShaderResourceView * CTextureMgr::CreateRandomTexture1DSRV(ID3D11Device * pd3dDevice)
{
	XMFLOAT4 RandomValue[1024];
	for (int i = 0; i < 1024; ++i)
		RandomValue[i] = XMFLOAT4(Chae::RandomFloat(-1.0f, 1.0f), Chae::RandomFloat(-1.0f, 1.0f), Chae::RandomFloat(-1.0f, 1.0f), Chae::RandomFloat(-1.0f, 1.0f));
	D3D11_SUBRESOURCE_DATA d3dSubresourceData;
	d3dSubresourceData.pSysMem = RandomValue;
	d3dSubresourceData.SysMemPitch = sizeof(XMFLOAT4) * 1024;
	d3dSubresourceData.SysMemSlicePitch = 0;

	D3D11_TEXTURE1D_DESC d3dTextureDesc;
	ZeroMemory(&d3dTextureDesc, sizeof(D3D11_TEXTURE1D_DESC));
	d3dTextureDesc.Width = 1024;
	d3dTextureDesc.MipLevels = 1;
	d3dTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	d3dTextureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	d3dTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	d3dTextureDesc.ArraySize = 1;

	ID3D11Texture1D * pd3dTexture;
	pd3dDevice->CreateTexture1D(&d3dTextureDesc, &d3dSubresourceData, &pd3dTexture);

	ID3D11ShaderResourceView * pd3dsrvTexutre;
	pd3dDevice->CreateShaderResourceView(pd3dTexture, nullptr, &pd3dsrvTexutre);
	pd3dTexture->Release();
	return(pd3dsrvTexutre);
}

//////////////////////////////////////////////////////////////////////////////////////////////
CMaterial::CMaterial()
{
	m_nReferences = 0;
	m_uMaterialSet = SET_SHADER_NONE;
}

CMaterial::~CMaterial()
{
}



CMaterialMgr::CMaterialMgr() : CMgr<CMaterial>()
{

}

CMaterialMgr::~CMaterialMgr()
{

}


CMaterialMgr & CMaterialMgr::GetInstance()
{
	static CMaterialMgr instance;
	return instance;
}

void CMaterialMgr::BuildResources(ID3D11Device * pd3dDevice)
{
	//재질을 생성한다.
	CMaterial *pMaterial = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	pMaterial->m_Material.m_xcAmbient = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(1.0f, 1.0f, 1.0f, 5.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	InsertObject(pMaterial, "Red");

	pMaterial = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	pMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(1.0f, 1.0f, 1.0f, 3.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	InsertObject(pMaterial, "Green");

	pMaterial = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	pMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(1.0f, 1.0f, 1.0f, 4.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	InsertObject(pMaterial, "Blue");

	pMaterial = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	pMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(1.0f, 1.0f, 1.0f, 4.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	InsertObject(pMaterial, "White");
}
