#pragma once
#include "ShaderType.h"

class CSceneShader : public CTexturedShader
{
	CTexture * m_pTexture;
	CTexture * m_pInfoScene;

	CMesh * m_pMesh;
	ID3D11DepthStencilState * m_pd3dDepthStencilState;
	
	int m_iDrawOption;
	ID3D11ShaderResourceView ** m_ppd3dMrtSrv;

	ID3D11PixelShader * m_pd3dPSOther;

	ID3D11ShaderResourceView * m_pd3dShadowSrv;
	ID3D11PixelShader * m_pd3dLightPS;
public:
	CSceneShader();
	virtual ~CSceneShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice, ID3D11ShaderResourceView ** ppd3dMrtSrv, int nMrtSrv);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	
	void SetTexture(int index, ID3D11ShaderResourceView * m_pSceneSRV);
	void SetLightSRV(ID3D11ShaderResourceView * pSRV)
	{
		m_pd3dShadowSrv = pSRV;
	}
	void SetDrawOption(int iOpt) { m_iDrawOption = iOpt; }
	int GetDrawOption() { return m_iDrawOption; }
};

class CShadowShader : public CShader
{
public:
	CShadowShader();
	virtual ~CShadowShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
};

class CPlayerShader : public CTexturedIlluminatedShader
{
public:
	CPlayerShader();
	virtual ~CPlayerShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain * pTerrain);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);

	CPlayer *GetPlayer(int nIndex = 0) { return((CPlayer *)m_ppObjects[nIndex]); }
};


class CWaterShader : public CTexturedShader
{
	ID3D11BlendState * m_pd3dWaterBlendState;

public:
	CWaterShader();
	virtual ~CWaterShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice, CHeightMapTerrain *pHeightMapTerrain);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);

private:
	void	SetBlendState(ID3D11Device *pd3dDevice);
};

class CTerrainShader : public CSplatLightingShader
{
	int m_nLayerNumber;
	CTexture ** m_pptxLayerMap;
public:
	CTerrainShader();
	virtual ~CTerrainShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera);
	CHeightMapTerrain *GetTerrain();
};


class CSkyBoxShader : public CTexturedShader
{
public:
	CSkyBoxShader();
	virtual ~CSkyBoxShader();

	virtual void BuildObjects(ID3D11Device *pd3dDevice);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	virtual void RenderReflected(ID3D11DeviceContext *pd3dDeviceContext, XMMATRIX *xmtxReflect, CCamera *pCamera = nullptr);
	virtual void CreateShader(ID3D11Device *pd3dDevice);
};
