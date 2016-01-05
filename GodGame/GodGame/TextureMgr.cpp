#include "stdafx.h"
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

void CTexture::UpdateTextureShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, int nIndex, int nSlot)
{
	pd3dDeviceContext->PSSetShaderResources(nSlot, 1, &m_ppd3dsrvTextures[nIndex]);
}

void CTexture::UpdateSamplerShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, int nIndex, int nSlot)
{
	pd3dDeviceContext->PSSetSamplers(nSlot, 1, &m_ppd3dSamplerStates[nIndex]);
}



CTextureMgr::CTextureMgr()
{
}


CTextureMgr::~CTextureMgr()
{
	for (auto it = m_vpTextureArray.begin(); it != m_vpTextureArray.end(); ++it)
	{
		if(it->second) 
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


}

void CTextureMgr::UpdateShaderVariable(ID3D11DeviceContext * pd3dDeviceContext, string name)
{
	if (m_vpTextureArray[name]) m_vpTextureArray[name]->UpdateShaderVariable(pd3dDeviceContext);
}

bool CTextureMgr::InsertShaderResourceView(ID3D11ShaderResourceView * pSRV, string name, UINT uSlotNum, SETSHADER nSetInfo)
{
	if (nullptr == pSRV) return false;

	CTexture * pTexture = new CTexture(1, 0, 0, 0, nSetInfo);
	pTexture->SetTexture(uSlotNum, pSRV);
	m_vpTextureArray[name] = pTexture;
	return true;
}

bool CTextureMgr::InsertSamplerState(ID3D11SamplerState * pSamplerState, string name, UINT uSlotNum, SETSHADER nSetInfo)
{
	if (nullptr == pSamplerState) return false;

	CTexture * pTexture = new CTexture(0, 1 , 0, 0, nSetInfo);
	pTexture->SetSampler(uSlotNum, pSamplerState);
	m_vpTextureArray[name] = pTexture;
	return true;
}

void CTextureMgr::EraseTexture(string name)
{
	if(m_vpTextureArray[name]) m_vpTextureArray[name]->Release();
	m_vpTextureArray[name] = nullptr;
}
