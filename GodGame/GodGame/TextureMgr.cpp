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



CTextureMgr::CTextureMgr()
{
}


CTextureMgr::~CTextureMgr()
{
	for (auto it = m_vpTextureArray.begin(); it != m_vpTextureArray.end(); ++it)
	{
		it->second->Release();
	}
}

CTextureMgr & CTextureMgr::GetInstance()
{
	static CTextureMgr instance;
	return instance;
	// TODO: 여기에 반환 구문을 삽입합니다.
}

bool CTextureMgr::InsertTexture(CTexture * pTexture, string name)
{
	if (nullptr == pTexture)
	{
		printf("텍스쳐가 존재하지 않습니다.");
		return false;
	}

	if (m_vpTextureArray[name])
	{
		m_vpTextureArray[name]->Release();
		m_vpTextureArray[name] = pTexture;
	}
	else
	{
		m_vpTextureArray[name] = pTexture;
	}

	pTexture->AddRef();

	return true;
}

CTexture * CTextureMgr::GetTexture(string name)
{
	return m_vpTextureArray[name];
}

void CTextureMgr::BuildResources(ID3D11Device * pd3dDevice)
{
	ID3D11SamplerState *pd3dSamplerState = nullptr;
	D3D11_SAMPLER_DESC d3dSamplerDesc;
	ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	d3dSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = 0;
	pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState);

	InsertSamplerState(pd3dSamplerState, "ss_linear_wrap", 0);
	pd3dSamplerState->Release();


	ID3D11ShaderResourceView *pd3dsrvTexture = nullptr;
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Miscellaneous/Brick02.jpg"), nullptr, nullptr, &pd3dsrvTexture, nullptr);
	if (FAILED(hr)) printf("오류");

	InsertShaderResourceView(pd3dsrvTexture, "srv_brick2_jpg", 0);
	pd3dsrvTexture->Release();


	d3dSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	d3dSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	d3dSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState);
	InsertSamplerState(pd3dSamplerState, "ss_linear_clamp", 0);
	pd3dSamplerState->Release();

	pd3dsrvTexture = CTextureMgr::CreateRandomTexture1DSRV(pd3dDevice);
	InsertShaderResourceView(pd3dsrvTexture, "srv_random1d", SLOT_RANDOM1D, SET_SHADER_GS | SET_SHADER_PS);
	pd3dsrvTexture->Release();
}

void CTextureMgr::UpdateShaderVariable(ID3D11DeviceContext * pd3dDeviceContext, string name)
{
	CTexture * pTexture = m_vpTextureArray[name];
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
	m_vpTextureArray[name] = pTexture;
	return true;
}

bool CTextureMgr::InsertSamplerState(ID3D11SamplerState * pSamplerState, string name, UINT uSlotNum, SETSHADER nSetInfo)
{
	if (nullptr == pSamplerState) return false;

	CTexture * pTexture = new CTexture(0, 1, 0, uSlotNum, nSetInfo);
	pTexture->SetSampler(0, pSamplerState);
	m_vpTextureArray[name] = pTexture;
	return true;
}

void CTextureMgr::EraseTexture(string name)
{
	if(m_vpTextureArray[name]) m_vpTextureArray[name]->Release();
	m_vpTextureArray[name] = nullptr;
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