#pragma once

#include "stdafx.h"
#include "MgrType.h"

#ifndef __SHADOW_MGR
#define __SHADOW_MGR

//ID3D11Resource
//ID3D11View

class CShadowMgr
{
private:
	int m_iStaticMapSlot;
	int m_iMapSlot;

	ID3D11DepthStencilView * m_pd3dDSVShadowMap;
	ID3D11ShaderResourceView * m_pd3dSRVShadowMap;

	ID3D11DepthStencilView * m_pd3dStaticDSVShadowMap;
	ID3D11ShaderResourceView * m_pd3dStaticSRVShadowMap;

	D3D11_VIEWPORT m_d3dxShadowMapViewport;
	ID3D11Buffer * m_pd3dcbShadowMap;

	XMFLOAT4X4 m_xmf44ShadowMap;
	XMFLOAT4X4 m_xmf44ShadowVP;
	ID3D11SamplerState * m_pd3dShadowSamplerState;

	ID3D11RasterizerState * m_pd3dShadowRS;
	ID3D11RasterizerState * m_pd3dNormalRS;


private:
	CShadowMgr();
	~CShadowMgr();

public:
	static CShadowMgr& GetInstance();

};
CShadowMgr::CShadowMgr()
{
	m_iStaticMapSlot = TX_SLOT_STATIC_SHADOW;
	m_iMapSlot = TX_SLOT_SHADOWMAP;

	m_pd3dStaticDSVShadowMap = m_pd3dDSVShadowMap = nullptr;
	m_pd3dStaticSRVShadowMap = m_pd3dSRVShadowMap = nullptr;

	ZeroMemory(&m_d3dxShadowMapViewport, sizeof(D3D11_VIEWPORT));
	m_pd3dcbShadowMap = nullptr;
	m_pd3dShadowSamplerState = nullptr;
	m_pd3dShadowRS = nullptr;
	m_pd3dNormalRS = nullptr;
}

CShadowMgr::~CShadowMgr()
{

}

CShadowMgr & CShadowMgr::GetInstance()
{
	static CShadowMgr instance;
	return instance;
}
#endif