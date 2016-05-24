#include "stdafx.h"
#include "MyInline.h"
#include "ResourceMgr.h"
#include "Shader.h"
//#include "Mesh.h"

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
	for (int i = 0; i < m_nTextures; i++) 
		if (m_ppd3dsrvTextures[i]) 
			m_ppd3dsrvTextures[i]->Release();
	for (int i = 0; i < m_nSamplers; i++) 
		if (m_ppd3dSamplerStates[i]) 
			m_ppd3dSamplerStates[i]->Release();
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

ID3D11ShaderResourceView *CTexture::CreateTexture2DArraySRV(ID3D11Device * pd3dDevice, const wchar_t * ppstrFilePaths, const wchar_t * ppstrFormat, UINT nTextures)
{
	D3DX11_IMAGE_LOAD_INFO d3dxImageLoadInfo;
	d3dxImageLoadInfo.Width          = D3DX11_FROM_FILE;
	d3dxImageLoadInfo.Height         = D3DX11_FROM_FILE;
	d3dxImageLoadInfo.Depth          = D3DX11_FROM_FILE;
	d3dxImageLoadInfo.FirstMipLevel  = 0;
	d3dxImageLoadInfo.MipLevels      = D3DX11_FROM_FILE;
	d3dxImageLoadInfo.Usage          = D3D11_USAGE_STAGING;
	d3dxImageLoadInfo.BindFlags      = 0;
	d3dxImageLoadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	d3dxImageLoadInfo.MiscFlags      = 0;
	d3dxImageLoadInfo.Format         = DXGI_FORMAT_FROM_FILE; //DXGI_FORMAT_R8G8B8A8_UNORM
	d3dxImageLoadInfo.Filter         = D3DX11_FILTER_NONE;
	d3dxImageLoadInfo.MipFilter      = D3DX11_FILTER_LINEAR;
	d3dxImageLoadInfo.pSrcInfo       = 0;

	ID3D11Texture2D **ppd3dTextures = new ID3D11Texture2D*[nTextures];
	_TCHAR pstrTextureName[80];
	for (UINT i = 0; i < nTextures; i++)
	{
		_stprintf_s(pstrTextureName, 80, _T("%s%02d.%s"), ppstrFilePaths, i + 1, ppstrFormat);
		D3DX11CreateTextureFromFile(pd3dDevice, pstrTextureName, &d3dxImageLoadInfo, 0, (ID3D11Resource **)&ppd3dTextures[i], 0);
	}
	D3D11_TEXTURE2D_DESC d3dTexure2DDesc;
	ppd3dTextures[0]->GetDesc(&d3dTexure2DDesc);

	D3D11_TEXTURE2D_DESC d3dTexture2DArrayDesc;
	d3dTexture2DArrayDesc.Width              = d3dTexure2DDesc.Width;
	d3dTexture2DArrayDesc.Height             = d3dTexure2DDesc.Height;
	d3dTexture2DArrayDesc.MipLevels          = d3dTexure2DDesc.MipLevels;
	d3dTexture2DArrayDesc.ArraySize          = nTextures;
	d3dTexture2DArrayDesc.Format             = d3dTexure2DDesc.Format;
	d3dTexture2DArrayDesc.SampleDesc.Count   = 1;
	d3dTexture2DArrayDesc.SampleDesc.Quality = 0;
	d3dTexture2DArrayDesc.Usage              = D3D11_USAGE_DEFAULT;
	d3dTexture2DArrayDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
	d3dTexture2DArrayDesc.CPUAccessFlags     = 0;
	d3dTexture2DArrayDesc.MiscFlags          = 0;

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
	d3dTextureSRVDesc.Format                         = d3dTexture2DArrayDesc.Format;
	d3dTextureSRVDesc.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	d3dTextureSRVDesc.Texture2DArray.MostDetailedMip = 0;
	d3dTextureSRVDesc.Texture2DArray.MipLevels       = d3dTexture2DArrayDesc.MipLevels;
	d3dTextureSRVDesc.Texture2DArray.FirstArraySlice = 0;
	d3dTextureSRVDesc.Texture2DArray.ArraySize       = nTextures;

	ID3D11ShaderResourceView *pd3dsrvTextureArray;
	pd3dDevice->CreateShaderResourceView(pd3dTexture2DArray, &d3dTextureSRVDesc, &pd3dsrvTextureArray);

	if (pd3dTexture2DArray) pd3dTexture2DArray->Release();

	for (UINT i = 0; i < nTextures; i++)
		if (ppd3dTextures[i])
			ppd3dTextures[i]->Release();
	delete[] ppd3dTextures;

	if (pd3dDeviceContext) pd3dDeviceContext->Release();

	return(pd3dsrvTextureArray);
}

CTextureMgr::CTextureMgr() : TEXTURE_MGR()
{
}

CTextureMgr::~CTextureMgr()
{
}

CTextureMgr & CTextureMgr::GetInstance()
{
	static CTextureMgr instance;
	return instance;
}

void CTextureMgr::BuildResources(ID3D11Device * pd3dDevice)
{
	BuildSamplers(pd3dDevice);
	BuildTextures(pd3dDevice);
}

void CTextureMgr::BuildSamplers(ID3D11Device * pd3dDevice)
{
	ID3D11SamplerState *pd3dSamplerState = nullptr;
	D3D11_SAMPLER_DESC d3dSamplerDesc;
	ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	{
		d3dSamplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		d3dSamplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
		d3dSamplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
		d3dSamplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
		d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		d3dSamplerDesc.MinLOD         = 0;
		d3dSamplerDesc.MaxLOD         = 0;

		ASSERT_S(pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState));
		InsertSamplerState(pd3dSamplerState, "ss_linear_wrap", 0);
		pd3dSamplerState->Release();
	}
	{
		d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		ASSERT_S(pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState));
		InsertSamplerState(pd3dSamplerState, "ss_point_wrap", 0);
		pd3dSamplerState->Release();
	}
	{
		d3dSamplerDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR; // D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		ASSERT_S(pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState));
		InsertSamplerState(pd3dSamplerState, "ss_linear_point_wrap", 0);
		pd3dSamplerState->Release();
	}
	{
		d3dSamplerDesc.Filter   = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

		ASSERT_S(pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState));
		InsertSamplerState(pd3dSamplerState, "ss_linear_clamp", 0);
		pd3dSamplerState->Release();
	}
	{
		d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		ASSERT_S(pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState));
		InsertSamplerState(pd3dSamplerState, "ss_point_clamp", 0);
		pd3dSamplerState->Release();
	}
	{
		ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
		d3dSamplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_BORDER;
		d3dSamplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_BORDER;
		d3dSamplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_BORDER;
		d3dSamplerDesc.Filter         = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		d3dSamplerDesc.MaxAnisotropy  = 1;
		d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;//D3D11_COMPARISON_NEVER;

		ASSERT_S(pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState));
		InsertSamplerState(pd3dSamplerState, "scs_point_border", 0);
		pd3dSamplerState->Release();
	}
	{
		d3dSamplerDesc.Filter         = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		d3dSamplerDesc.MaxAnisotropy  = 1;

		ASSERT_S(pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState));
		InsertSamplerState(pd3dSamplerState, "ss_point_border", 0);
		pd3dSamplerState->Release();
	}
}

void CTextureMgr::BuildTextures(ID3D11Device * pd3dDevice)
{
	HRESULT hr = 0;
	ID3D11ShaderResourceView *pd3dsrvTexture = nullptr;
	
	string name;
	string directory;
	wstring wsdir;

	ifstream in;
	in.open("../Assets/Data/Resource/SRV.txt", ios::in);
//	wchar_t wsdirName[128];

	while (in >> name >> directory)
	{
		wsdir.assign(directory.begin(), directory.end());
		ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, wsdir.c_str(), nullptr, nullptr, &pd3dsrvTexture, nullptr));
		InsertShaderResourceView(pd3dsrvTexture, name, 0);
		pd3dsrvTexture->Release();
	}
	in.close();

	wstring extensionName;
	int level;
	in.open("../Assets/Data/Resource/2DSRVArray.txt", ios::in);
	while (in >> name >> directory)
	{
		wsdir.assign(directory.begin(), directory.end());
		
		in >> directory >> level;
		extensionName.assign(directory.begin(), directory.end());

		pd3dsrvTexture = CTexture::CreateTexture2DArraySRV(pd3dDevice, wsdir.c_str(), extensionName.c_str(), level);
		InsertShaderResourceView(pd3dsrvTexture, name, 0);
		pd3dsrvTexture->Release();
	}
	in.close();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CTextureMgr::UpdateShaderVariable(ID3D11DeviceContext * pd3dDeviceContext, string name)
{
	CTexture * pTexture = m_mpList[name];
	if (pTexture)
	{
		if (pTexture->IsSampler() && pTexture->IsSRV()) pTexture->UpdateShaderVariable(pd3dDeviceContext);
		else if (pTexture->IsSRV()) pTexture->UpdateTextureShaderVariable(pd3dDeviceContext);
		else if (pTexture->IsSampler()) pTexture->UpdateSamplerShaderVariable(pd3dDeviceContext);
	}
}

CTexture * CTextureMgr::MakeFbxcjhTextures(ID3D11Device * pd3dDevice, wstring & wstrBaseDir, vector<wstring>& vcFileNameArrays, int nStartTxSlot)
{
	ID3D11ShaderResourceView * pd3dsrvTexture = nullptr;

	CTexture * pTexture = new CTexture(vcFileNameArrays.size(), 1, nStartTxSlot, 0);
	pTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));

	int index = 0;
	for (int i = 0; i < vcFileNameArrays.size(); ++i, ++index)
	{
		vcFileNameArrays[index] = wstrBaseDir + vcFileNameArrays[i];
		LPCWSTR wstr = vcFileNameArrays[index].c_str();

		ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, vcFileNameArrays[index].c_str(), nullptr, nullptr, &pd3dsrvTexture, nullptr));
		pTexture->SetTexture(index, pd3dsrvTexture);
		pd3dsrvTexture->Release();
	}

	return pTexture;
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

//////////////////////////////////////////////////////////////////////////////////////////////
CMaterial::CMaterial()
{
	m_nReferences = 0;
	m_uMaterialSet = SET_SHADER_NONE;
}

CMaterial::~CMaterial()
{
}

CMaterialMgr::CMaterialMgr() : MATERIAL_MGR()
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
	CMaterial *pMaterial               = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse  = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	pMaterial->m_Material.m_xcAmbient  = XMFLOAT4(0.2f, 0.0f, 0.0f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(1.0f, 1.0f, 1.0f, 5.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	InsertObject(pMaterial, "Red");

	pMaterial                          = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse  = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	pMaterial->m_Material.m_xcAmbient  = XMFLOAT4(0.0f, 0.2f, 0.0f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(1.0f, 1.0f, 1.0f, 3.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	InsertObject(pMaterial, "Green");

	pMaterial                          = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse  = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	pMaterial->m_Material.m_xcAmbient  = XMFLOAT4(0.0f, 0.0f, 0.2f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(1.0f, 1.0f, 1.0f, 4.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	InsertObject(pMaterial, "Blue");

	pMaterial                          = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse  = XMFLOAT4(0.8f, 0.8f, 0.8f, 0.8f);
	pMaterial->m_Material.m_xcAmbient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(1.5f, 1.5f, 1.5f, 4.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	InsertObject(pMaterial, "White");

	pMaterial = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 0.7f);
	pMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(1.0f, 1.0f, 1.0f, 3.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	InsertObject(pMaterial, "PlayerWhite");

	pMaterial = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse = XMFLOAT4(0.8f, 0.2f, 0.2f, 0.7f);
	pMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.8f, 0.1f, 0.1f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(0.9f, 0.2f, 0.2f, 3.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	InsertObject(pMaterial, "PlayerRed");

	pMaterial = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse = XMFLOAT4(0.2f, 0.8f, 0.2f, 0.7f);
	pMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.1f, 0.8f, 0.1f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(0.2f, 0.9f, 0.2f, 3.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	InsertObject(pMaterial, "PlayerGreen");

	pMaterial = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse = XMFLOAT4(0.2f, 0.2f, 0.8f, 0.7f);
	pMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.1f, 0.1f, 0.8f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(0.2f, 0.2f, 0.3f, 3.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	InsertObject(pMaterial, "PlayerBlue");

	pMaterial = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse = XMFLOAT4(0.2f, 0.7f, 0.7f, 0.7f);
	pMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.1f, 0.7f, 0.7f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(0.2f, 0.3f, 0.3f, 3.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	InsertObject(pMaterial, "PlayerCyan");

	pMaterial = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse = XMFLOAT4(0.7f, 0.7f, 0.2f, 0.7f);
	pMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.7f, 0.7f, 0.1f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(0.3f, 0.3f, 0.2f, 3.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	InsertObject(pMaterial, "PlayerYellow");

	pMaterial = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse = XMFLOAT4(0.7f, 0.2f, 0.7f, 0.7f);
	pMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.7f, 0.1f, 0.7f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(0.3f, 0.2f, 0.3f, 3.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	InsertObject(pMaterial, "PlayerMagenta");

	pMaterial = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse = XMFLOAT4(3.0f, 3.0f, 3.0f, 0.8f);
	pMaterial->m_Material.m_xcAmbient = XMFLOAT4(2.0f, 2.0f, 2.0f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(2.0f, 2.0f, 2.0f, 4.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	InsertObject(pMaterial, "WhiteLight");

	pMaterial = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse = XMFLOAT4(4.0f, 0.0f, 0.0f, 0.8f);
	pMaterial->m_Material.m_xcAmbient = XMFLOAT4(3.0f, 0.0f, 0.0f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(3.0f, 0.0f, 0.0f, 4.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
	InsertObject(pMaterial, "RedLight");

	pMaterial = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse = XMFLOAT4(0.0f, 3.0f, 0.0f, 0.8f);
	pMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.0f, 2.0f, 0.0f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(0.0f, 2.0f, 0.0f, 4.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	InsertObject(pMaterial, "GreenLight");

	pMaterial = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse = XMFLOAT4(0.0f, 0.0f, 4.0f, 0.8f);
	pMaterial->m_Material.m_xcAmbient = XMFLOAT4(0.0f, 0.0f, 3.0f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(0.0f, 0.0f, 3.0f, 4.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	InsertObject(pMaterial, "BlueLight");

	pMaterial                          = new CMaterial();
	pMaterial->m_Material.m_xcDiffuse  = XMFLOAT4(0.3f, 0.5f, 0.3f, 1.0f);
	pMaterial->m_Material.m_xcAmbient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	pMaterial->m_Material.m_xcSpecular = XMFLOAT4(1.0f, 3.0f, 1.0f, 5.0f);
	pMaterial->m_Material.m_xcEmissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	InsertObject(pMaterial, "Terrain");
}
#ifdef _USE_MESH_MGR
CMeshMgr::CMeshMgr() : CMgr<CMesh>()
{
}

CMeshMgr::~CMeshMgr()
{
}

CMeshMgr & CMeshMgr::GetInstance()
{
	static CMeshMgr instance;
	return instance;
}

void CMeshMgr::BuildResources(ID3D11Device * pd3dDevice)
{
	vector<wstring> vcTxFileNames;
	wstring baseDir{ _T("../Assets/Image/Objects/") };

	//CLoadMeshByChae *pCubeMesh = new CLoadMeshByChae(pd3dDevice, ("../Assets/Image/Objects/Medicbag.chae"), 2.0f);//new CCubeMeshTexturedIlluminated(pd3dDevice, 12.0f, 12.0f, 12.0f);

	CLoadMeshByFbxcjh *pCubeMesh = new CLoadMeshByFbxcjh(pd3dDevice, ("../Assets/Image/Objects/staff/Staff01.fbxcjh"), 0.2f, vcTxFileNames);

	vcTxFileNames.clear();
}
#endif

ID3D11ShaderResourceView * CViewManager::CreateRandomTexture1DSRV(ID3D11Device * pd3dDevice)
{
	XMFLOAT4 RandomValue[1024];
	for (int i = 0; i < 1024; ++i)
		RandomValue[i] = XMFLOAT4(Chae::RandomFloat(-1.0f, 1.0f), Chae::RandomFloat(-1.0f, 1.0f), Chae::RandomFloat(-1.0f, 1.0f), Chae::RandomFloat(-1.0f, 1.0f));
	D3D11_SUBRESOURCE_DATA d3dSubresourceData;
	d3dSubresourceData.pSysMem          = RandomValue;
	d3dSubresourceData.SysMemPitch      = sizeof(XMFLOAT4) * 1024;
	d3dSubresourceData.SysMemSlicePitch = 0;

	D3D11_TEXTURE1D_DESC d3dTextureDesc;
	ZeroMemory(&d3dTextureDesc, sizeof(D3D11_TEXTURE1D_DESC));
	d3dTextureDesc.Width                = 1024;
	d3dTextureDesc.MipLevels            = 1;
	d3dTextureDesc.Format               = DXGI_FORMAT_R32G32B32A32_FLOAT;
	d3dTextureDesc.Usage                = D3D11_USAGE_IMMUTABLE;
	d3dTextureDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE;
	d3dTextureDesc.ArraySize            = 1;

	ID3D11Texture1D * pd3dTexture;
	pd3dDevice->CreateTexture1D(&d3dTextureDesc, &d3dSubresourceData, &pd3dTexture);

	ID3D11ShaderResourceView * pd3dsrvTexutre;
	pd3dDevice->CreateShaderResourceView(pd3dTexture, nullptr, &pd3dsrvTexutre);
	pd3dTexture->Release();
	return(pd3dsrvTexutre);
}

CViewManager & CViewManager::GetInstance()
{
	static CViewManager instance;
	return instance;
}

void CViewManager::ReleaseResources()
{
	m_mgrSrv.ReleaseObjects();
	m_mgrRtv.ReleaseObjects();
	m_mgrDsv.ReleaseObjects();
	m_mgrUav.ReleaseObjects();
	m_mgrBuffer.ReleaseObjects();
	m_mgrTex2D.ReleaseObjects();
}

void CViewManager::BuildResources(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext)
{
	BuildViews(pd3dDevice, pd3dDeviceContext);
	BuildConstantBuffers(pd3dDevice, pd3dDeviceContext);
}

void CViewManager::BuildViews(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext)
{
	CreatePostProcessViews(pd3dDevice, pd3dDeviceContext);
	CreateViewsInGame(pd3dDevice, pd3dDeviceContext);

	ID3D11ShaderResourceView *pd3dsrvTexture = nullptr;
	pd3dsrvTexture = CViewManager::CreateRandomTexture1DSRV(pd3dDevice);
	InsertSRV(pd3dsrvTexture, "srv_random1d");// TX_SLOT_RANDOM1D, SET_SHADER_GS | SET_SHADER_PS);
	pd3dsrvTexture->Release();
}

void CViewManager::CreatePostProcessViews(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext)
{
	HRESULT hResult = 0;
	ID3D11DepthStencilView	  * pDSV = nullptr;
	ID3D11ShaderResourceView  * pSRV = nullptr;
	ID3D11UnorderedAccessView * pUAV = nullptr;
	ID3D11RenderTargetView    * pRTV = nullptr;

	ID3D11Texture2D * pTx2D = nullptr;
	ID3D11Buffer    * pBuffer1, *pBuffer2, *pBuffer3;
	pBuffer1 = pBuffer2 = pBuffer3 = nullptr;

	D3D11_RENDER_TARGET_VIEW_DESC d3dRTVDesc;
	d3dRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	d3dRTVDesc.Texture2D.MipSlice = 0;
	d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D11_TEXTURE2D_DESC d3d2DBufferDesc;
	ZeroMemory(&d3d2DBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));
	d3d2DBufferDesc.Width              = FRAME_BUFFER_WIDTH;
	d3d2DBufferDesc.Height             = FRAME_BUFFER_HEIGHT;
	d3d2DBufferDesc.MipLevels          = 1;
	d3d2DBufferDesc.ArraySize          = 1;
	d3d2DBufferDesc.Format             = DXGI_FORMAT_R32_TYPELESS;//DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3d2DBufferDesc.SampleDesc.Count   = 1;
	d3d2DBufferDesc.SampleDesc.Quality = 0;
	d3d2DBufferDesc.Usage              = D3D11_USAGE_DEFAULT;
	d3d2DBufferDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;// | D3D11_BIND_SHADER_RESOURCE;
	d3d2DBufferDesc.CPUAccessFlags     = 0;
	d3d2DBufferDesc.MiscFlags          = 0;
	//ASSERT(SUCCEEDED(hResult         = pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &m_pd3dDepthStencilBuffer/*m_ppd3dMRTtx[MRT_DEPTH]*/)));

	//생성한 깊이 버퍼(Depth Buffer)에 대한 뷰를 생성한다.
	D3D11_DEPTH_STENCIL_VIEW_DESC d3dViewDesc;
	ZeroMemory(&d3dViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	d3dViewDesc.Format = DXGI_FORMAT_D32_FLOAT;//d3d2DBufferDesc.Format;
	d3dViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	d3dViewDesc.Texture2D.MipSlice = 0;
	//ASSERT(SUCCEEDED((hResult        = m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dViewDesc, &m_pd3dDepthStencilView))));

	D3D11_SHADER_RESOURCE_VIEW_DESC d3dSRVDesc;
	ZeroMemory(&d3dSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	d3dSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	d3dSRVDesc.Texture2D.MipLevels = 1;
	d3dSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	//if (FAILED(hResult               = m_pd3dDevice->CreateShaderResourceView(m_ppd3dMRTtx[MRT_DEPTH], &d3dSRVDesc, &m_pd3dMRTSRV[MRT_DEPTH])))
	//	return(false);

	d3d2DBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

	D3D11_UNORDERED_ACCESS_VIEW_DESC d3dUAVDesc;
	ZeroMemory(&d3dUAVDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	d3dUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	d3dUAVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dUAVDesc.Texture2D.MipSlice = 0;

	d3dUAVDesc.Format = d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3d2DBufferDesc.Width = FRAME_BUFFER_WIDTH * 0.25f;
	d3d2DBufferDesc.Height = FRAME_BUFFER_HEIGHT * 0.25f;
	{
		ASSERT_S(hResult = pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &pTx2D));
		ASSERT_S(hResult = pd3dDevice->CreateShaderResourceView(pTx2D, &d3dSRVDesc, &pSRV));
		ASSERT_S(hResult = pd3dDevice->CreateUnorderedAccessView(pTx2D, &d3dUAVDesc, &pUAV));
		if (pTx2D) pTx2D->Release();

		InsertSRV(pSRV, "su2d_post0"); pSRV->Release();
		InsertUAV(pUAV, "su2d_post0"); pUAV->Release();
	}
	{
		ASSERT_S(hResult = pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &pTx2D));
		ASSERT_S(hResult = pd3dDevice->CreateShaderResourceView(pTx2D, &d3dSRVDesc, &pSRV));
		ASSERT_S(hResult = pd3dDevice->CreateUnorderedAccessView(pTx2D, &d3dUAVDesc, &pUAV));
		if (pTx2D) pTx2D->Release();

		InsertSRV(pSRV, "su2d_post1"); pSRV->Release();
		InsertUAV(pUAV, "su2d_post1"); pUAV->Release();
	}
	d3d2DBufferDesc.Width = FRAME_BUFFER_WIDTH * 0.0625f;
	d3d2DBufferDesc.Height = FRAME_BUFFER_HEIGHT * 0.0625f;
	{
		ASSERT_S(hResult = pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &pTx2D));
		ASSERT_S(hResult = pd3dDevice->CreateShaderResourceView(pTx2D, &d3dSRVDesc, &pSRV));
		ASSERT_S(hResult = pd3dDevice->CreateUnorderedAccessView(pTx2D, &d3dUAVDesc, &pUAV));
		if (pTx2D) pTx2D->Release();

		InsertSRV(pSRV, "su2d_postscaled0"); pSRV->Release();
		InsertUAV(pUAV, "su2d_postscaled0"); pUAV->Release();
	}
	{
		ASSERT_S(hResult = pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &pTx2D));
		ASSERT_S(hResult = pd3dDevice->CreateShaderResourceView(pTx2D, &d3dSRVDesc, &pSRV));
		ASSERT_S(hResult = pd3dDevice->CreateUnorderedAccessView(pTx2D, &d3dUAVDesc, &pUAV));
		if (pTx2D) pTx2D->Release();

		InsertSRV(pSRV, "su2d_postscaled1"); pSRV->Release();
		InsertUAV(pUAV, "su2d_postscaled1"); pUAV->Release();
	}
	d3d2DBufferDesc.Width = FRAME_BUFFER_WIDTH * 0.25f;
	d3d2DBufferDesc.Height = FRAME_BUFFER_HEIGHT * 0.25f;
	d3dRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	d3d2DBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	{
		ASSERT_S(hResult = pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &pTx2D));
		ASSERT_S(hResult = pd3dDevice->CreateShaderResourceView(pTx2D, &d3dSRVDesc, &pSRV));
		ASSERT_S(hResult = pd3dDevice->CreateRenderTargetView(pTx2D, &d3dRTVDesc, &pRTV));
		if (pTx2D) pTx2D->Release();

		InsertSRV(pSRV, "sr2d_bloom4x4"); pSRV->Release();
		InsertRTV(pRTV, "sr2d_bloom4x4"); pRTV->Release();
	}
	d3d2DBufferDesc.Width = FRAME_BUFFER_WIDTH * 0.0625;//0.125f;
	d3d2DBufferDesc.Height = FRAME_BUFFER_HEIGHT * 0.0625;//0.125f;
	{
		ASSERT_S(hResult = pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &pTx2D));
		ASSERT_S(hResult = pd3dDevice->CreateShaderResourceView(pTx2D, &d3dSRVDesc, &pSRV));
		ASSERT_S(hResult = pd3dDevice->CreateRenderTargetView(pTx2D, &d3dRTVDesc, &pRTV));
		if (pTx2D) pTx2D->Release();

		InsertSRV(pSRV, "sr2d_bloom16x16"); pSRV->Release();
		InsertRTV(pRTV, "sr2d_bloom16x16"); pRTV->Release();
	}
	d3d2DBufferDesc.Width = FRAME_BUFFER_WIDTH;
	d3d2DBufferDesc.Height = FRAME_BUFFER_HEIGHT;

	d3dRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	d3d2DBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;// | D3D11_BIND_UNORDERED_ACCESS;
	d3d2DBufferDesc.Format = d3dSRVDesc.Format = d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	{
		ASSERT_S(hResult = pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &pTx2D));
		ASSERT_S(hResult = pd3dDevice->CreateShaderResourceView(pTx2D, &d3dSRVDesc, &pSRV));
		ASSERT_S(hResult = pd3dDevice->CreateRenderTargetView(pTx2D, &d3dRTVDesc, &pRTV));
		if (pTx2D) pTx2D->Release();

		InsertSRV(pSRV, "sr2d_PostResult"); pSRV->Release();
		InsertRTV(pRTV, "sr2d_PostResult"); pRTV->Release();
	}
	d3d2DBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	{
		ASSERT_S(hResult = pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &pTx2D));
		ASSERT_S(hResult = pd3dDevice->CreateShaderResourceView(pTx2D, &d3dSRVDesc, &pSRV));
		ASSERT_S(hResult = pd3dDevice->CreateUnorderedAccessView(pTx2D, &d3dUAVDesc, &pUAV));
		if (pTx2D) pTx2D->Release();

		InsertSRV(pSRV, "su2d_radial"); pSRV->Release();
		InsertUAV(pUAV, "su2d_radial"); pUAV->Release();
	}
	//{
	//	ASSERT(SUCCEEDED(hResult = pd3dDevice->CreateTexture2D(&d3d2DBufferDesc, nullptr, &pTx2D)));
	//	ASSERT(SUCCEEDED(hResult = pd3dDevice->CreateShaderResourceView(pTx2D, &d3dSRVDesc, &pSRV)));
	//	ASSERT(SUCCEEDED(hResult = pd3dDevice->CreateRenderTargetView(pTx2D, &d3dRTVDesc, &pRTV)));
	//	if (pTx2D) pTx2D->Release();

	//	InsertSRV(pSRV, "sr2d_SSAO"); pSRV->Release();
	//	InsertRTV(pRTV, "sr2d_SSAO"); pRTV->Release();
	//}

	// Create two buffers for ping-ponging in the reduction operation used for calculating luminance
	D3D11_BUFFER_DESC DescBuffer;
	ZeroMemory(&DescBuffer, sizeof(D3D11_BUFFER_DESC));
	DescBuffer.BindFlags           = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	DescBuffer.ByteWidth           = int(ceil(FRAME_BUFFER_WIDTH / 8.0f) * ceil(FRAME_BUFFER_HEIGHT / 8.0f)) * sizeof(float);
	DescBuffer.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	DescBuffer.StructureByteStride = sizeof(float);
	DescBuffer.Usage               = D3D11_USAGE_DEFAULT;
	{
		ASSERT_S(pd3dDevice->CreateBuffer(&DescBuffer, nullptr, &pBuffer1));
		ASSERT_S(pd3dDevice->CreateBuffer(&DescBuffer, nullptr, &pBuffer2));
	}
	//{
	//	// This Buffer is for reduction on CPU
	//	DescBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	//	DescBuffer.Usage = D3D11_USAGE_STAGING;
	//	DescBuffer.BindFlags = 0;
	//	ASSERT(SUCCEEDED(pd3dDevice->CreateBuffer(&DescBuffer, nullptr, &m_pd3dComputeRead)));
	//}
	// Create UAV on the above two buffers object
	D3D11_UNORDERED_ACCESS_VIEW_DESC DescUAV;
	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format              = DXGI_FORMAT_UNKNOWN;
	DescUAV.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
	DescUAV.Buffer.FirstElement = 0;
	DescUAV.Buffer.NumElements  = DescBuffer.ByteWidth / sizeof(float);
	{
		ASSERT_S(pd3dDevice->CreateUnorderedAccessView(pBuffer1, &DescUAV, &pUAV));
		InsertUAV(pUAV, "su_reduce1"); pUAV->Release();
		ASSERT_S(pd3dDevice->CreateUnorderedAccessView(pBuffer2, &DescUAV, &pUAV));
		InsertUAV(pUAV, "su_reduce2"); pUAV->Release();
	}
	// Create resource view for the two buffers object
	D3D11_SHADER_RESOURCE_VIEW_DESC DescRV;
	ZeroMemory(&DescRV, sizeof(DescRV));
	DescRV.Format              = DXGI_FORMAT_UNKNOWN;
	DescRV.ViewDimension       = D3D11_SRV_DIMENSION_BUFFER;
	DescRV.Buffer.FirstElement = DescUAV.Buffer.FirstElement;
	DescRV.Buffer.NumElements  = DescUAV.Buffer.NumElements;
	{
		ASSERT_S(pd3dDevice->CreateShaderResourceView(pBuffer1, &DescRV, &pSRV));
		InsertSRV(pSRV, "su_reduce1"); pSRV->Release();
		ASSERT_S(pd3dDevice->CreateShaderResourceView(pBuffer2, &DescRV, &pSRV));
		InsertSRV(pSRV, "su_reduce2"); pSRV->Release();
	}
	//{
	//	DescBuffer.ByteWidth = sizeof(float) * 16;
	//	DescUAV.Buffer.NumElements = DescBuffer.ByteWidth / sizeof(float);
	//	DescRV.Buffer.FirstElement = DescUAV.Buffer.FirstElement;
	//	DescRV.Buffer.NumElements  = DescUAV.Buffer.NumElements;

	//	ASSERT(SUCCEEDED(pd3dDevice->CreateBuffer(&DescBuffer, nullptr, &pBuffer3)));
	//	ASSERT(SUCCEEDED(pd3dDevice->CreateUnorderedAccessView(pBuffer3, &DescUAV, &pUAV)));
	//	InsertUAV(pUAV, "su_4last_reduce"); pUAV->Release();
	//	ASSERT(SUCCEEDED(pd3dDevice->CreateShaderResourceView(pBuffer3, &DescRV, &pSRV)));
	//	InsertSRV(pSRV, "su_4last_reduce"); pSRV->Release();
	//}
	pBuffer1->Release();
	pBuffer2->Release();
	//pBuffer3->Release();
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Buffers for blooming effect in CS path
	//ZeroMemory(&DescBuffer, sizeof(DescBuffer));
	//DescBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	//DescBuffer.ByteWidth = FRAME_BUFFER_WIDTH / 8 * FRAME_BUFFER_HEIGHT / 8 * sizeof(XMFLOAT4);
	//DescBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	//DescBuffer.StructureByteStride = sizeof(XMFLOAT4);
	//DescBuffer.Usage = D3D11_USAGE_DEFAULT;

	//ZeroMemory(&DescRV, sizeof(DescRV));
	//DescRV.Format = DXGI_FORMAT_UNKNOWN;
	//DescRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	//DescRV.Buffer.FirstElement = 0;
	//DescRV.Buffer.NumElements = DescBuffer.ByteWidth / DescBuffer.StructureByteStride;

	//ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	//DescUAV.Format = DXGI_FORMAT_UNKNOWN;
	//DescUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	//DescUAV.Buffer.FirstElement = 0;
	//DescUAV.Buffer.NumElements = DescRV.Buffer.NumElements;

	//string name = "su_bloom";
	//for (int i = 0; i < NUM_BLOOM_TEXTURES; i++)
	//{
	//	ASSERT(SUCCEEDED(pd3dDevice->CreateBuffer(&bufdesc, nullptr, &m_csBloom.m_pd3dCBBufferArray[i])));
	//	ASSERT(SUCCEEDED(pd3dDevice->CreateShaderResourceView(m_csBloom.m_pd3dCBBufferArray[i], &DescRV, &m_csBloom.m_pd3dSRVArray[i])));
	//	ASSERT(SUCCEEDED(pd3dDevice->CreateUnorderedAccessView(m_csBloom.m_pd3dCBBufferArray[i], &DescUAV, &m_csBloom.m_pd3dUAVArray[i])));
	//}
}

void CViewManager::CreateViewsInGame(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext)
{
	HRESULT hResult = 0;
	ID3D11DepthStencilView	  * pDSV = nullptr;
	ID3D11ShaderResourceView  * pSRV = nullptr;
	ID3D11UnorderedAccessView * pUAV = nullptr;
	ID3D11RenderTargetView    * pRTV = nullptr;

	ID3D11Texture2D * pTx2D = nullptr;
	ID3D11Buffer    * pBuffer1, *pBuffer2, *pBuffer3;
	pBuffer1 = pBuffer2 = pBuffer3 = nullptr;

	D3D11_RENDER_TARGET_VIEW_DESC d3dRTVDesc;
	d3dRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	d3dRTVDesc.Texture2D.MipSlice = 0;
	d3dRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D11_TEXTURE2D_DESC d3d2DBufferDesc;
	ZeroMemory(&d3d2DBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));
	d3d2DBufferDesc.Width              = FRAME_BUFFER_WIDTH;
	d3d2DBufferDesc.Height             = FRAME_BUFFER_HEIGHT;
	d3d2DBufferDesc.MipLevels          = 1;
	d3d2DBufferDesc.ArraySize          = 1;
	d3d2DBufferDesc.Format             = DXGI_FORMAT_R32_TYPELESS;//DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3d2DBufferDesc.SampleDesc.Count   = 1;
	d3d2DBufferDesc.SampleDesc.Quality = 0;
	d3d2DBufferDesc.Usage              = D3D11_USAGE_DEFAULT;
	d3d2DBufferDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;// | D3D11_BIND_SHADER_RESOURCE;
	d3d2DBufferDesc.CPUAccessFlags     = 0;
	d3d2DBufferDesc.MiscFlags          = 0;
}

void CViewManager::BuildConstantBuffers(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext)
{
	HRESULT hr = 0;
	ID3D11Buffer * pd3dBuffer = nullptr;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(XMFLOAT4);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;// 0;
	ASSERT_S(hr = pd3dDevice->CreateBuffer(&desc, nullptr, &pd3dBuffer));

	InsertBuffer(pd3dBuffer, "cs_float4");
	pd3dBuffer->Release();

	desc.ByteWidth = sizeof(XMFLOAT4X4);
	ASSERT_S(hr = pd3dDevice->CreateBuffer(&desc, nullptr, &pd3dBuffer));
	InsertBuffer(pd3dBuffer, "cs_float4x4");
	pd3dBuffer->Release();

	desc.ByteWidth = sizeof(CB_PARTICLE);
	ASSERT_S(pd3dDevice->CreateBuffer(&desc, nullptr, &pd3dBuffer));
	InsertBuffer(pd3dBuffer, "cs_particle");
	pd3dBuffer->Release();

	desc.ByteWidth = sizeof(CB_TX_ANIMATION_INFO);
	ASSERT_S(pd3dDevice->CreateBuffer(&desc, nullptr, &pd3dBuffer));
	InsertBuffer(pd3dBuffer, "cs_tx_animation");
	pd3dBuffer->Release();
}

void MapConstantBuffer(ID3D11DeviceContext * pd3dDeviceContext, void * data, size_t size, ID3D11Buffer * pBuffer)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	memcpy(d3dMappedResource.pData, data, size);
	pd3dDeviceContext->Unmap(pBuffer, 0);
}

void MapMatrixConstantBuffer(ID3D11DeviceContext * pd3dDeviceContext, XMFLOAT4X4 & matrix, ID3D11Buffer * pBuffer)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	XMFLOAT4X4 *pcbWorldMatrix = (XMFLOAT4X4*)d3dMappedResource.pData;
	Chae::XMFloat4x4Transpose(pcbWorldMatrix, &matrix); //XMFLOAT4X4Transpose(&pcbWorldMatrix->m_d3dxTransform, pxmtxWorld);
	pd3dDeviceContext->Unmap(pBuffer, 0);
}

CFileLoadManager& CFileLoadManager::GetInstance()
{
	static CFileLoadManager instance;
	return instance;
}

void CFileLoadManager::LoadSceneResources(ID3D11Device * pd3dDevice, void * resourceMgr, const char * fileName)
{
	CShader::BUILD_RESOURCES_MGR * sceneResoucres = static_cast<CShader::BUILD_RESOURCES_MGR *>(resourceMgr);
	static CTexture * pTexture = nullptr;
	static CMesh * pMesh = nullptr;

	//wstring baseDir{ _T("../Assets/Image/Objects/") };
	vector<wstring> vcTxFileNames;
	ID3D11ShaderResourceView *pd3dsrvTexture = nullptr;

	ifstream in;
	in.open(fileName, ios::in);

	string objName, instruction;
	while (in >> objName)
	{
		if (0 == objName.compare("name"))
			in >> objName;

		int st = 0, end = 1;
		float scale = 1.f;
		vector<wstring> txNameArray;
		string meshName;

		while (true)
		{
			in >> instruction;
			if (0 == instruction.compare("for"))
			{
				in >> st >> end;
			}
			if (0 == instruction.compare("tx"))
			{
				int txNum;
				in >> txNum;

				string txName;
				for (int i = 0; i < txNum; ++i)
				{
					in >> txName;
					txNameArray.emplace_back();
					(txNameArray.end() - 1)->assign(txName.begin(), txName.end());
				}
			}
			if (0 == instruction.compare("fbxcjh"))
			{
				in >> scale;
				string mesh;
				in >> meshName;
			}

			if (0 == instruction.compare("end"))
			{
				static char file[128];
				static wchar_t result[128];

				for (int i = st; i < end; ++i)
				{
					pTexture = new CTexture(txNameArray.size(), 1, 0, 0);
					for (int j = 0; j < txNameArray.size(); ++j)
					{
						swprintf(result, txNameArray[j].c_str(), i);
						ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
						pTexture->SetTexture(j, pd3dsrvTexture);
						pd3dsrvTexture->Release();
					}
					sprintf(file, meshName.c_str(), i);
					pMesh = new CLoadMeshByFbxcjh(pd3dDevice, file, scale, vcTxFileNames);
					vcTxFileNames.clear();

					sprintf(file, objName.c_str(), i);
					sceneResoucres->mgrTexture.InsertObject(pTexture, file);
					sceneResoucres->mgrMesh.InsertObject(pMesh, file);
				}
				break;
			}
		}

	}
	in.close();
}

void CFileLoadManager::LoadSceneAnimationObjects(ID3D11Device * pd3dDevice, void * resourceMgr, const char * fileName)
{
	CShader::BUILD_RESOURCES_MGR * sceneResoucres = static_cast<CShader::BUILD_RESOURCES_MGR *>(resourceMgr);
	static CTexture * pTexture = nullptr;
	static CMesh * pMesh = nullptr;

	const static wstring baseDir{ _T("../Assets/Image/Objects/") };
	vector<wstring> vcTxFileNames;

	ifstream in;
	in.open(fileName, ios::in);

	wstring wsdir;
	string objName, instruction;
	while (in >> instruction)
	{
		in >> objName;	// "name"
		in >> instruction >> instruction; // "dir"
		wsdir.assign(instruction.begin(), instruction.end());

		string animName;
		string adDir;
		float scale = 1.0f;
		int st = 0, end = 0;
		int index = 0;

		while(true)
		{
			in >> animName;
			if (0 == animName.compare("end")) break;

			in >> adDir >> scale >> st >> end;

			pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, adDir.c_str(), scale, vcTxFileNames, st, end);
			sceneResoucres->mgrMesh.InsertObject(pMesh, animName);
			if (0 == index) 
			{
				pTexture = CTextureMgr::MakeFbxcjhTextures(pd3dDevice, baseDir + wsdir, vcTxFileNames, 0);
				vcTxFileNames.clear();
				sceneResoucres->mgrTexture.InsertObject(pTexture, objName);
			}
			++index;
		}
		vcTxFileNames.clear();

	}
	in.close();
}