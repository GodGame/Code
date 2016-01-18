#pragma once
#include "ShaderType.h"

#define NUM_SSAO_OFFSET 14

static float GaussianDistribution(float x, float y, float rho);

#define NUM_TONEMAP_TEXTURES  5       // Number of stages in the 3x3 down-scaling for post-processing in PS
static const int ToneMappingTexSize = (int)pow(4.0f, NUM_TONEMAP_TEXTURES - 1);
#define NUM_BLOOM_TEXTURES 2

struct CB_SSAO_INFO
{
	XMFLOAT4X4 m_gViewToTexSpace; // 투영 행렬 * 텍스쳐 행렬
	XMFLOAT4 m_gOffsetVectors[NUM_SSAO_OFFSET];
	XMFLOAT4 m_gFrustumCorners[4];
};

// Constant buffer layout for transferring data to the PS
struct CB_PS
{
	float param[4];
};

struct CB_CS
{
	UINT param[4];	// x, y = dispatch , z , w = input size;
};

struct CB_CS_BLOOM
{
	XMFLOAT2   m_uInputSize;
	XMFLOAT2   m_uOutputSize;
	float  m_fInverse;
	float  m_fThreshold;
	XMFLOAT2 m_fExtra;
	//XMFLOAT4  m_fParam;	// x = inverse
	//UINT   m_uInputSize[2];
	//UINT    m_uOutputwidth;
	//float   m_fInverse;
};
#define SLOT_CS_CB_BLOOM 2 

struct CB_WEIGHT
{
	XMFLOAT4 fWeight[11];
//	XMFLOAT4 fInoutSize;
//	float fInverse;
};
#define SLOT_CS_CB_WEIGHT 1

struct CB_CS_INOUT
{

};

struct POST_CS
{
	ID3D11ComputeShader * m_pd3dComputeShader;
	ID3D11UnorderedAccessView * m_pd3dUAV;
	ID3D11ShaderResourceView * m_pd3dSRV;
	ID3D11Buffer * m_pd3dCBBuffer;
};

template <int num>
class POST_CS_NUM
{
public:
	ID3D11ComputeShader * m_pd3dComputeShader;
	ID3D11UnorderedAccessView * m_pd3dUAVArray[num];
	ID3D11ShaderResourceView * m_pd3dSRVArray[num];
	ID3D11Buffer * m_pd3dCBBufferArray[num];

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
	CB_WEIGHT m_cbWeights;

	CTexture * m_pTexture;
	CTexture * m_pInfoScene;

	CMesh * m_pMesh;
	ID3D11DepthStencilState * m_pd3dDepthStencilState;
	
	int m_iDrawOption;
	ID3D11ShaderResourceView ** m_ppd3dMrtSrv;

	float m_fInverseToneTex;

private:
	ID3D11Buffer * m_pd3dCBWeight;

	ID3D11PixelShader * m_pd3dPSFinal;
	ID3D11PixelShader * m_pd3dPSOther;
	ID3D11PixelShader * m_pd3dLightPS;
	ID3D11PixelShader * m_pd3dPSDump;

	ID3D11ShaderResourceView * m_pd3dShadowSrv;

	ID3D11RenderTargetView * m_pd3dBackRTV;
	
	ID3D11RenderTargetView * m_pd3dBloom4x4RTV;
	ID3D11ShaderResourceView * m_pd3dBloom4x4SRV;

	ID3D11UnorderedAccessView * m_pd3dPostUAV[2];
	ID3D11ShaderResourceView * m_pd3dPostSRV[2];
	ID3D11ComputeShader * m_pd3dComputeHorzBlur;
	ID3D11ComputeShader * m_pd3dComputeVertBlur;
	ID3D11ComputeShader * m_pd3dComputeHorzBloom;
	ID3D11ComputeShader * m_pd3dComputeVertBloom;

	ID3D11Buffer * m_pd3dCBComputeInfo;
	ID3D11Buffer * m_pd3dComputeRead;
	ID3D11Buffer * m_pd3dCBBloomInfo;

	ID3D11ComputeShader * m_pd3dCSReduceToSingle;
	
	POST_CS_REPEATABLE m_csReduce;
	POST_CS_BLOOMING m_csBloom;

public:
	CSceneShader();
	virtual ~CSceneShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice, ID3D11ShaderResourceView ** ppd3dMrtSrv, int nMrtSrv, ID3D11RenderTargetView * pd3dBackRTV);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	
public:
	void FinalRender(ID3D11DeviceContext *pd3dDeviceContext, ID3D11ShaderResourceView * pBloomSRV, UINT uRenderState, CCamera *pCamera = nullptr);
	void MeasureLuminance(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	void SceneBlur(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	void Blooming(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	void DumpMap(ID3D11DeviceContext *pd3dDeviceContext, ID3D11ShaderResourceView * pSRVsource, ID3D11RenderTargetView * pRTVTarget, DWORD dFrameWidth, DWORD dFrameHeight, CCamera * pCamera);

public:
	void SetPostView(ID3D11ShaderResourceView ** ppd3dPostSrv, ID3D11UnorderedAccessView ** ppd3dPostUav);
	void SetTexture(int index, ID3D11ShaderResourceView * m_pSceneSRV);
	void SetInfoTextures(ID3D11DeviceContext *pd3dDeviceContext);

	void SetBloomScaled(ID3D11RenderTargetView * pd3dRTV, ID3D11ShaderResourceView * pd3dSRV);

	void SetLightSRV(ID3D11ShaderResourceView * pSRV) { m_pd3dShadowSrv = pSRV; }
	void SetDrawOption(int iOpt) { m_iDrawOption = iOpt; }
	int GetDrawOption() { return m_iDrawOption; }

public:
	void UpdateShaders(ID3D11DeviceContext *pd3dDeviceContext);
	void CreateConstantBuffer(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext);
	void UpdateShaderReosurces(ID3D11DeviceContext *pd3dDeviceContext);

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
