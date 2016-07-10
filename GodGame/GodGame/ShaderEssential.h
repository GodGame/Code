#pragma once

#ifndef __SHADER_ESSTENTIAL
#define __SHADER_ESSTENTIAL

#include "ShaderType.h"

#define NUM_SSAO_OFFSET 14

static float GaussianDistribution(float x, float y, float rho);

#define NUM_TONEMAP_TEXTURES  5       // Number of stages in the 3x3 down-scaling for post-processing in PS
static const int ToneMappingTexSize = (int)pow(4.0f, NUM_TONEMAP_TEXTURES - 1);//128
#define NUM_BLOOM_TEXTURES	  2

struct CB_SSAO_INFO
{
	XMFLOAT4X4 m_gViewToTexSpace; // 투영 행렬 * 텍스쳐 행렬
	XMFLOAT4   m_gOffsetVectors[NUM_SSAO_OFFSET];
	XMFLOAT4   m_gFrustumCorners[4];
};

struct CB_CS_BLOOM
{
	XMFLOAT2    m_uInputSize;
	XMFLOAT2    m_uOutputSize;
	float       m_fInverse;
	float       m_fThreshold;
	XMFLOAT2    m_fExtra;
	//XMFLOAT4  m_fParam;	// x = inverse
	//UINT      m_uInputSize[2];
	//UINT      m_uOutputwidth;
	//float     m_fInverse;
};
#define SLOT_CS_CB_BLOOM 2

struct CB_WEIGHT
{
	XMFLOAT4 fWeight[11];
	//	XMFLOAT4 fInoutSize;
	//	float fInverse;
};
#define SLOT_CS_CB_WEIGHT 1

struct CB_WATER
{
	float  fTime;
	float  fTimePerMoveUnit;
	float  fEmpty;
	float  fWaterDepth;
};

struct POST_CS
{
	ID3D11ComputeShader       * m_pd3dComputeShader;
	ID3D11UnorderedAccessView * m_pd3dUAV;
	ID3D11ShaderResourceView  * m_pd3dSRV;
	ID3D11Buffer              * m_pd3dCBBuffer;
};

struct POST_CS_VIEWS
{
	ID3D11ComputeShader       * m_pd3dComputeShader;
	ID3D11UnorderedAccessView * m_pd3dUAV;
	ID3D11ShaderResourceView  * m_pd3dSRV;
	ID3D11RenderTargetView    * m_pd3dRTV;

	POST_CS_VIEWS()
	{
		m_pd3dComputeShader = nullptr;
		m_pd3dUAV           = nullptr;
		m_pd3dSRV           = nullptr;
		m_pd3dRTV           = nullptr;
	}
	~POST_CS_VIEWS()
	{
		if (m_pd3dComputeShader) m_pd3dComputeShader->Release();
		if (m_pd3dUAV) m_pd3dUAV->Release();
		if (m_pd3dSRV) m_pd3dSRV->Release();
		if (m_pd3dRTV) m_pd3dRTV->Release();
	}
};

template <int num>
class POST_CS_NUM
{
public:
	ID3D11ComputeShader       * m_pd3dComputeShader;
	ID3D11UnorderedAccessView * m_pd3dUAVArray[num];
	ID3D11ShaderResourceView  * m_pd3dSRVArray[num];
	ID3D11Buffer              * m_pd3dCBBufferArray[num];

	void ReleaseObjects()
	{
		if (m_pd3dComputeShader) m_pd3dComputeShader->Release();

		for (int i = 0; i < num; ++i)
		{
			if (m_pd3dCBBufferArray[i])m_pd3dCBBufferArray[i]->Release();
			if (m_pd3dSRVArray[i]) m_pd3dSRVArray[i]->Release();
			if (m_pd3dUAVArray[i]) m_pd3dUAVArray[i]->Release();
		}
	}
	POST_CS_NUM<num>()
	{
		m_pd3dComputeShader = nullptr;

		for (int i = 0; i < num; ++i)
		{
			m_pd3dCBBufferArray[i] = nullptr;
			m_pd3dSRVArray[i] = nullptr;
			m_pd3dUAVArray[i] = nullptr;
		}
	}

	virtual ~POST_CS_NUM<num>()
	{
		if (m_pd3dComputeShader) m_pd3dComputeShader->Release();

		for (int i = 0; i < num; ++i)
		{
			if (m_pd3dCBBufferArray[i])m_pd3dCBBufferArray[i]->Release();
			if (m_pd3dSRVArray[i]) m_pd3dSRVArray[i]->Release();
			if (m_pd3dUAVArray[i]) m_pd3dUAVArray[i]->Release();
		}
	}
	virtual void swap(int one, int two)
	{
		std::swap(m_pd3dCBBufferArray[one], m_pd3dCBBufferArray[two]);
		std::swap(m_pd3dUAVArray[one], m_pd3dUAVArray[two]);
		std::swap(m_pd3dSRVArray[one], m_pd3dSRVArray[two]);;
	}
};

class POST_CS_REPEATABLE : public POST_CS_NUM<2>
{
public:
	POST_CS_REPEATABLE() : POST_CS_NUM<2>() {  };
	virtual ~POST_CS_REPEATABLE() {};
};

class POST_CS_BLOOMING : public POST_CS_NUM<NUM_BLOOM_TEXTURES>
{
public:
	POST_CS_BLOOMING() : POST_CS_NUM<NUM_BLOOM_TEXTURES>() {};
	virtual ~POST_CS_BLOOMING() {};
};

class CSceneShader : public CTexturedShader
{
	float     m_fFrameTime;
	float     m_fTotalTime;

	CB_WEIGHT m_cbWeights;

	CTexture                * m_pTexture;
	CTexture                * m_pInfoScene;

	CMesh                   * m_pMesh;
	ID3D11DepthStencilState * m_pd3dDepthStencilState;

	int m_iDrawOption;
	ID3D11ShaderResourceView ** m_ppd3dMrtSrv;

	float m_fInverseToneTex;

private:
	ID3D11Buffer      * m_pd3dCBWeight;

	ID3D11PixelShader * m_pd3dPSFinal;
	ID3D11PixelShader * m_pd3dPSOther;
	ID3D11PixelShader * m_pd3dLightPS;
	ID3D11PixelShader * m_pd3dPSDump;

	//ID3D11ShaderResourceView * m_pd3dShadowSrv;

	ID3D11RenderTargetView     * m_pd3dBackRTV;

	ID3D11RenderTargetView     * m_pd3dBloom4x4RTV;
	ID3D11ShaderResourceView   * m_pd3dBloom4x4SRV;
	ID3D11RenderTargetView     * m_pd3dBloom16x16RTV;
	ID3D11ShaderResourceView   * m_pd3dBloom16x16SRV;

	ID3D11UnorderedAccessView  * m_pd3dPostUAV[2];
	ID3D11ShaderResourceView   * m_pd3dPostSRV[2];
	ID3D11UnorderedAccessView  * m_pd3dPostScaledUAV[2];
	ID3D11ShaderResourceView   * m_pd3dPostScaledSRV[2];
	// 블룸 및 블러 자원들
	ID3D11ComputeShader        * m_pd3dComputeHorzBlur;
	ID3D11ComputeShader        * m_pd3dComputeVertBlur;
	ID3D11ComputeShader        * m_pd3dComputeHorzBloom;
	ID3D11ComputeShader        * m_pd3dComputeVertBloom;
	
	ID3D11Buffer               * m_pd3dCBComputeInfo;
	ID3D11Buffer               * m_pd3dCBBloomInfo;
	// 광적응 자원들
	ID3D11ComputeShader        * m_pd3dCSAdaptLum;
	ID3D11ComputeShader        * m_pd3dCSReduceToSingle;
	ID3D11ShaderResourceView   * m_pd3dLastReducedSRV;
	ID3D11UnorderedAccessView  * m_pd3dLastReducedUAV;
	POST_CS_REPEATABLE           m_csReduce;
	// Final을 걸쳐 그려질 View들
	ID3D11RenderTargetView     * m_pd3dPostRenderRTV;
	ID3D11ShaderResourceView   * m_pd3dPostRenderSRV;

	ID3D11ComputeShader		   * m_pd3dCSRadialBlur;
	ID3D11UnorderedAccessView  * m_pd3dRadialUAV;
	ID3D11ShaderResourceView   * m_pd3dRadialSRV;

	//POST_CS_VIEWS				m_csRadialBlur;

public:
	CSceneShader();
	virtual ~CSceneShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice, ID3D11ShaderResourceView ** ppd3dMrtSrv, int nMrtSrv, ID3D11RenderTargetView * pd3dBackRTV);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	virtual void AnimateObjects(float fTimeElapsed);
	void PostProcessingRender(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);

	virtual void GetGameMessage(CShader * byObj, eMessage eMSG, void * extra = nullptr);
	virtual void SendGameMessage(CShader * toObj, eMessage eMSG, void * extra = nullptr);

public:
	void ScreenRender    (ID3D11DeviceContext *pd3dDeviceContext, ID3D11ShaderResourceView * pScreenSRV, UINT uRenderState);
	void HDRFinalRender  (ID3D11DeviceContext *pd3dDeviceContext, ID3D11ShaderResourceView * pBloomSRV[], UINT uRenderState, CCamera *pCamera = nullptr);
	void MeasureLuminance(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	void SceneBlur       (ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	void Blooming        (ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	void DumpMap         (ID3D11DeviceContext *pd3dDeviceContext, ID3D11ShaderResourceView * pSRVsource, ID3D11RenderTargetView * pRTVTarget, DWORD dFrameWidth, DWORD dFrameHeight, CCamera * pCamera);
	void RadialBlur      (ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);

public:
	void SetTexture(int index, ID3D11ShaderResourceView * m_pSceneSRV);
	void SetInfoTextures(ID3D11DeviceContext *pd3dDeviceContext);
	void SetDrawOption(int iOpt) { m_iDrawOption = iOpt; }
	int GetDrawOption() { return m_iDrawOption; }

public:
	void UpdateShaders(ID3D11DeviceContext *pd3dDeviceContext);
	void CreateConstantBuffer(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext);
	void UpdateShaderReosurces(ID3D11DeviceContext *pd3dDeviceContext);
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

class CPlayerShader : public CShader
{
#define PLAYER_INDEX 0
	int m_iPlayerIndex;

public:
	CPlayerShader();
	virtual ~CPlayerShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice, CShader::BUILD_RESOURCES_MGR & mgrScene, CScene* pScene);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void Reset();

	void RoundStart();
	void RoundEnd();

	void SetPlayerID(ID3D11Device * pd3dDevice, int id);
	CPlayer *GetPlayer(int nIndex) { return static_cast<CPlayer *>(m_ppObjects[nIndex]); }
	CPlayer *GetPlayer() { return static_cast<CPlayer *>(m_ppObjects[m_iPlayerIndex]); }
};

class CWaterShader : public CTexturedShader
{
	ID3D11BlendState * m_pd3dWaterBlendState;
	ID3D11Buffer * m_pd3dcbWaterBuffer;
	CB_WATER mCBWaterData;
#define CB_WATER_SLOT 0x03

public:
	CWaterShader();
	virtual ~CWaterShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	virtual void AnimateObjects(float fTimeElapsed);

private:
	void	_SetBlendState(ID3D11Device *pd3dDevice);
	void	_SetWaterCB(ID3D11DeviceContext *pd3dDeviceContext);
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
	//CHeightMapTerrain *GetTerrain();
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

class CUIShader : public CShader
{
protected:
	CScene				   * m_pScene;

	ID3D11RenderTargetView * m_pBackRTV;
	ID3D11BlendState       * m_pd3dBlendState;
	ID3D11Buffer           * m_pd3dScreenInfoBuffer;

	CGameObject			   * m_pMousePoint;

public:
	CUIShader();
	virtual ~CUIShader();

	virtual void OnPrepareRender(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState);
	virtual void BuildObjects(ID3D11Device *pd3dDevice, ID3D11RenderTargetView * pBackRTV, CScene * pScene);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	virtual void FontRender(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr){}
	virtual void CreateShader(ID3D11Device *pd3dDevice);

	void CreateUIResources(ID3D11Device * pd3dDevice);

public:
	void SetMouseCursor(POINT & pt) { if (m_pMousePoint) m_pMousePoint->SetPosition(XMFLOAT3(pt.x, pt.y, 0)); }
	void MouseDown(HWND hwnd, POINT & pt);
};

class CTitleScreenShader : public CUIShader
{
public:
	CTitleScreenShader();
	virtual ~CTitleScreenShader();

	virtual void BuildObjects(ID3D11Device *pd3dDevice, ID3D11RenderTargetView * pBackRTV, CScene * pScene);
	virtual void GetGameMessage(CShader * byObj, eMessage eMSG, void * extra = nullptr);
};

class CLobbyScreenShader : public CUIShader
{
public:
	CLobbyScreenShader();
	virtual ~CLobbyScreenShader();

	virtual void BuildObjects(ID3D11Device *pd3dDevice, ID3D11RenderTargetView * pBackRTV, CScene * pScene);
	virtual void GetGameMessage(CShader * byObj, eMessage eMSG, void * extra = nullptr);
};

class CInGameUIShader : public CUIShader
{
	bool   mbNeedElement : 1;

	static const int mfNeedElementTime = 3.f;

	const int miResultReverseIndex = 1;
	const int miItemUIReverseIndex = 2;
	const string mWinLogo = "win_logo";
	const string mLoseLogo = "lose_logo";

	CMgr<CTexture> mTextureList;

public:
	CInGameUIShader();
	virtual ~CInGameUIShader();

	virtual void BuildObjects(ID3D11Device *pd3dDevice, ID3D11RenderTargetView * pBackRTV, CScene * pScene);
	virtual void GetGameMessage(CShader * byObj, eMessage eMSG, void * extra = nullptr);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	virtual void FontRender(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	virtual void AnimateObjects(float fTimeElapsed);

	void DrawUserNames(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);

	void UIReadyWinLogo(bool Visible);
	void UIReadyLoseLogo(bool Visible);

	void ChangeItemUI(CStaff * pStaff);
};

#endif