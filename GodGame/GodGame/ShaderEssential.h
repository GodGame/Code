#pragma once
#include "ShaderType.h"

#define NUM_SSAO_OFFSET 14
struct CB_SSAO_INFO
{
	XMFLOAT4X4 m_gViewToTexSpace; // 투영 행렬 * 텍스쳐 행렬
	XMFLOAT4 m_gOffsetVectors[NUM_SSAO_OFFSET];
	XMFLOAT4 m_gFrustumCorners[4];
};

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
	void SetInfoTextures(ID3D11DeviceContext *pd3dDeviceContext);
	void UpdateShaders(ID3D11DeviceContext *pd3dDeviceContext);

	void SetLightSRV(ID3D11ShaderResourceView * pSRV) { m_pd3dShadowSrv = pSRV; }
	void SetDrawOption(int iOpt) { m_iDrawOption = iOpt; }
	int GetDrawOption() { return m_iDrawOption; }
};

class CShadowShader : public CTexturedShader
{
public:
	CShadowShader();
	virtual ~CShadowShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
};

class CSSAOShader : public CShader
{
	CMesh * m_pMesh;
	ID3D11Buffer *  m_pd3dcbSSAOInfo;
	CB_SSAO_INFO m_ssao;
	ID3D11ShaderResourceView * m_pd3dSRVSSAO;
public:
	CSSAOShader();
	virtual ~CSSAOShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);

	void BuildSSAO(ID3D11Device *pd3dDevice);
	void CreateShaderVariable(ID3D11Device *pd3dDevice);
	void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, CCamera * pCamera);
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
//	void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera);
	CHeightMapTerrain *GetTerrain();
};


class CSkyBoxShader : public CTexturedShader
{
public:
	CSkyBoxShader();
	virtual ~CSkyBoxShader();

	virtual void BuildObjects(ID3D11Device *pd3dDevice);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
		virtual void CreateShader(ID3D11Device *pd3dDevice);
};
