#include "stdafx.h"
#include "ParentShader.h"


CParentShader::CParentShader()
{
	m_nShaders = 0;
	m_ppShader = nullptr;
}

CParentShader::~CParentShader()
{
	if (m_ppShader)
	{
		for (int i = 0; i < m_nShaders; ++i)
			delete m_ppShader[i];
		delete[] m_ppShader;
	}
}

void CParentShader::CreateShader(ID3D11Device * pd3dDevice)
{
}

void CParentShader::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_nShaders; ++i)
	{
		m_ppShader[i]->AnimateObjects(fTimeElapsed);
	}
}

void CParentShader::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	for (int i = 0; i < m_nShaders; ++i)
	{
		m_ppShader[i]->Render(pd3dDeviceContext, uRenderState, pCamera);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEnviromentShader::CEnviromentShader() : CParentShader()
{
}

CEnviromentShader::~CEnviromentShader()
{
}

void CEnviromentShader::BuildObjects(ID3D11Device * pd3dDevice)
{
	m_nShaders = 2;
	m_ppShader = new CShader*[m_nShaders];

	int index = 0;
	m_ppShader[index] = new CSkyBoxShader();
	m_ppShader[index]->CreateShader(pd3dDevice);
	m_ppShader[index++]->BuildObjects(pd3dDevice);

	m_ppShader[index] = new CTerrainShader();
	m_ppShader[index]->CreateShader(pd3dDevice);
	m_ppShader[index++]->BuildObjects(pd3dDevice);
}

void CEnviromentShader::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	if (uRenderState & RS_SHADOWMAP)
		m_ppShader[1]->Render(pd3dDeviceContext, uRenderState, pCamera);
	else
		for (int i = 0; i < m_nShaders; ++i)
		{
			m_ppShader[i]->Render(pd3dDeviceContext, uRenderState, pCamera);
		}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEffectShader::CEffectShader() : CParentShader()
{
}

CEffectShader::~CEffectShader()
{
}

void CEffectShader::ShaderKeyEventOn(CPlayer * pPlayer, WORD key, void * extra)
{
	CInGamePlayer * pInGamePlayer = static_cast<CInGamePlayer*>(pPlayer);

	switch (key)
	{
	case 'B':
		((CParticleShader*)m_ppShader[mParticleEffectNum])->ParticleOn(4, pPlayer, &pPlayer->GetPosition(), &pPlayer->GetLookVector(), &pPlayer->GetLookVectorInverse());
		return;

	case 'X':
		((CTextureAniShader*)m_ppShader[mTxAniEffectNum])->EffectOn(1, pPlayer, &pPlayer->GetPosition(), &pPlayer->GetLookVector(), &pPlayer->GetLookVector());
		return;

	case 'C':
		((CTextureAniShader*)m_ppShader[mTxAniEffectNum])->EffectOn(2, pPlayer, &pPlayer->GetPosition(), &pPlayer->GetLookVector(), nullptr);
		return;
	}
}

void CEffectShader::BuildObjects(ID3D11Device * pd3dDevice)
{
	m_nShaders = 2;
	m_ppShader = new CShader*[m_nShaders];

	CTextureAniShader * pTxAni = new CTextureAniShader();
	pTxAni->CreateShader(pd3dDevice);
	pTxAni->BuildObjects(pd3dDevice, nullptr);
	m_ppShader[0] = pTxAni;

	CParticleShader * pParticleShader = new CParticleShader();
	pParticleShader->CreateShader(pd3dDevice);
	pParticleShader->BuildObjects(pd3dDevice, nullptr);
	m_ppShader[1] = pParticleShader;
}

void CEffectShader::GetGameMessage(CShader * byObj, eMessage eMSG, void * extra)
{
	XMFLOAT4 xmf4Data;
	CInGamePlayer * pPlayer = nullptr; static_cast<CInGamePlayer*>(extra);

	switch (eMSG)
	{
	case eMessage::MSG_PARTICLE_ON:
		memcpy(&xmf4Data, extra, sizeof(XMFLOAT4));
		static_cast<CParticleShader*>(m_ppShader[mParticleEffectNum])->ParticleOn(nullptr, (XMFLOAT3*)&xmf4Data, xmf4Data.w);
		return;

	case eMessage::MSG_MAGIC_SHOT:
		pPlayer = static_cast<CInGamePlayer*>(extra);
		static_cast<CParticleShader*>(m_ppShader[mParticleEffectNum])->ParticleOn(pPlayer->Get1HAnimShotParticleOnInfo());
		return;

	case eMessage::MSG_MAGIC_AREA:
		pPlayer = static_cast<CInGamePlayer*>(extra);
		((CTextureAniShader*)m_ppShader[mTxAniEffectNum])->EffectOn(0, pPlayer, &pPlayer->GetCenterPosition());
		return;
	}
}

CStaticModelingShader::CStaticModelingShader()
{
}

CStaticModelingShader::~CStaticModelingShader()
{
}

void CStaticModelingShader::BuildObjects(ID3D11Device * pd3dDevice, CMaterial * pMaterial, BUILD_RESOURCES_MGR & SceneMgr)
{
	m_nShaders = 3;
	m_ppShader = new CShader*[m_nShaders];

	CItemShader *pItemShader = new CItemShader();
	pItemShader->CreateShader(pd3dDevice);
	pItemShader->BuildObjects(pd3dDevice, pMaterial, SceneMgr);
	m_ppShader[0] = pItemShader;

	CStaticShader *pStaticShader = new CStaticShader();
	pStaticShader->CreateShader(pd3dDevice);
	pStaticShader->BuildObjects(pd3dDevice, pMaterial, SceneMgr);
	m_ppShader[1] = pStaticShader;

	CBlackAlphaShader *pBlackImageShader = new CBlackAlphaShader();
	pBlackImageShader->CreateShader(pd3dDevice);
	pBlackImageShader->BuildObjects(pd3dDevice, pMaterial, SceneMgr);
	m_ppShader[2] = pBlackImageShader;
}