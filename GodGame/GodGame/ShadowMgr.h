#pragma once


//#include "MgrType.h"

#ifndef __SHADOW_MGR
#define __SHADOW_MGR

#define SIZE_SHADOW_WIDTH 1024
#define SIZE_SHADOW_HEIGHT 1024
#define SHADOW_DEPTH_BIAS 5000

class CCamera;
class CShadowMgr
{
private:
	int m_iStaticMapSlot;
	int m_iMapSlot;
	XMFLOAT3 m_xmf3LightPos;

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
	CShadowMgr& operator=(const CShadowMgr&);

public:
	static CShadowMgr& GetInstance();

	void ReleaseDevices();
	void CreateShadowDevice(ID3D11Device * pd3dDevice);
	void BuildShadowMap(ID3D11DeviceContext * pd3dDeviceContext, XMFLOAT3 & Target, XMFLOAT3 & LightPos, float fHalf);

	void SetStaticShadowMap(ID3D11DeviceContext * pd3dDeviceContext, CCamera * pCamera);
	void ResetStaticShadowMap(ID3D11DeviceContext * pd3dDeviceContext, CCamera * pCamera);
	void UpdateStaticShadowResource(ID3D11DeviceContext * pd3dDeviceContext);

	void SetDynamicShadowMap(ID3D11DeviceContext * pd3dDeviceContext, CCamera * pCamera);
	void ResetDynamicShadowMap(ID3D11DeviceContext * pd3dDeviceContext, CCamera * pCamera);
	void UpdateDynamicShadowResource(ID3D11DeviceContext * pd3dDeviceContext);
};

#define ShadowMgr CShadowMgr::GetInstance()

#endif