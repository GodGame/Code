#pragma once

#include "ShaderType.h"

class CStaticShader : public CShader
{
	CMaterial * m_pMaterial;

public:
	CStaticShader();
	virtual ~CStaticShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice, CMaterial * pMaterial, BUILD_RESOURCES_MGR & SceneMgr);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);

	virtual void GetGameMessage(CShader * byObj, eMessage eMSG, void * extra = nullptr);
};

class CItemShader : public CShader, public CInstanceShader
{
	CMaterial * m_pMaterial;

	typedef vector<CItem*> ItemList;
//	typedef vector<CGameObject*> ItemList;
	vector<ItemList> m_vcItemList;

public:
	CItemShader();
	virtual ~CItemShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice, CMaterial * pMaterial, BUILD_RESOURCES_MGR & SceneMgr);
	
	virtual void AnimateObjects(float fTimeElapsed);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);

	virtual void GetGameMessage(CShader * byObj, eMessage eMSG, void * extra = nullptr);
};

class CCharacterShader : public CShader
{
	CMaterial * m_pMaterial;

public:
	CCharacterShader();
	virtual ~CCharacterShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice, CMaterial * pMaterial, BUILD_RESOURCES_MGR & SceneMgr);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	virtual void Reset();

	virtual void GetGameMessage(CShader * byObj, eMessage eMSG, void * extra = nullptr);
};

class CInstancingShader : public CInstanceShader, public CTexturedShader
{

public:
	CInstancingShader();
	~CInstancingShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);

	virtual void BuildObjects(ID3D11Device *pd3dDevice, CMaterial *pMaterial, CTexture *pTexture, int k);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);

private:
	CMaterial *m_pMaterial;
	CTexture  *m_pTexture;

	//UINT m_nInstanceBufferStride;
	//UINT m_nInstanceBufferOffset;

	ID3D11Buffer *m_pd3dCubeInstanceBuffer;
	ID3D11Buffer *m_pd3dSphereInstanceBuffer;

public:
	//인스턴싱 버퍼를 생성한다.
	//ID3D11Buffer *CreateInstanceBuffer(ID3D11Device *pd3dDevice, int nObjects, UINT nBufferStride, void *pBufferData);
};

class CStaticInstaningShader : public CShader, public CInstanceShader
{
private:
#define NUM_ROCK 4
	ID3D11Buffer * m_pd3dRockInstanceBuffer[NUM_ROCK];
	int m_nRocks[NUM_ROCK];
	CGameObject ** m_ppRockObjects[NUM_ROCK];
	CTexture *m_pRockTexture[NUM_ROCK];

#define NUM_STONE 3
	ID3D11Buffer * m_pd3dStoneInstanceBuffer[NUM_ROCK];
	int m_nStones[NUM_ROCK];
	CGameObject ** m_ppStoneObjects[NUM_ROCK];
	CTexture *m_pStoneTexture[NUM_ROCK];

public:
	CStaticInstaningShader();
	~CStaticInstaningShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);

	virtual void BuildObjects(ID3D11Device *pd3dDevice, CShader::BUILD_RESOURCES_MGR & mgrScene);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	virtual void AnimateObjects(float fTimeElapsed);
	void AllRender(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);

private:
	CMaterial * m_pMaterial;
};

class CBillboardShader : public CShader, public CInstanceShader
{
private:
	ID3D11Buffer * m_pd3dTreeInstanceBuffer;
	int m_nTrees;

public:
	CBillboardShader();
	~CBillboardShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);

	virtual void BuildObjects(ID3D11Device *pd3dDevice);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	virtual void AnimateObjects(float fTimeElapsed);
	void AllRender(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);

	static void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera);
private:
	CTexture *m_pTexture;

public:
	//인스턴싱 버퍼를 생성한다.
	//ID3D11Buffer *CreateInstanceBuffer(ID3D11Device *pd3dDevice, int nObjects, UINT nBufferStride, void *pBufferData);
};

class CPointInstanceShader : public CShader, public CInstanceShader
{
private:
	ID3D11Buffer * m_pd3dCubeInstanceBuffer;
	int m_nCubes;
public:
	CPointInstanceShader();
	~CPointInstanceShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);

	virtual void BuildObjects(ID3D11Device *pd3dDevice, CMaterial * pMaterial);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	virtual void AnimateObjects(float fTimeElapsed);
	//static void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, CCamera *pCamera);
private:
	CTexture  * m_pTexture;
	CMaterial * m_pMaterial;

public:
	//인스턴싱 버퍼를 생성한다.
	//ID3D11Buffer *CreateInstanceBuffer(ID3D11Device *pd3dDevice, int nObjects, UINT nBufferStride, void *pBufferData);
};

class CBlackAlphaShader : public CShader
{
public:
	CBlackAlphaShader();
	virtual ~CBlackAlphaShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice, CMaterial * pMaterial, BUILD_RESOURCES_MGR & SceneMgr);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
};

class CNormalShader : public CNormalMapShader
{
	CTexture  * m_pTexture;
	CMaterial * m_pMaterial;
public:
	CNormalShader();
	~CNormalShader();

	virtual void CreateShader(ID3D11Device *pd3dDevice);

	virtual void BuildObjects(ID3D11Device *pd3dDevice, CMaterial * pMaterial);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);

};
class CTextureAniShader : public CShader
{
	CTxAnimationObject ** m_ppEffctsObjects;
	ID3D11BlendState   *  m_pd3dBlendState;
	ID3D11SamplerState *  m_pd3dSamplerState;

public:
	CTextureAniShader();
	virtual ~CTextureAniShader();

public:	
	void CreateStates(ID3D11Device * pd3dDevice);
	virtual void CreateShader(ID3D11Device *pd3dDevice);

	virtual void BuildObjects(ID3D11Device *pd3dDevice, CMaterial * pMaterial);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	virtual void AnimateObjects(float fTimeElapsed);

public:
	bool SetEffect(int index, CGameObject * pObj, XMFLOAT3 * pos = nullptr)
	{
		if (m_ppEffctsObjects[index]->IsAble()) return false;
		return m_ppEffctsObjects[index]->Enable(pObj, pos);
	}
	CTxAnimationObject * GetEffect(int index) { return m_ppEffctsObjects[index]; }

	//void EffectOn(XMFLOAT3 * pos = nullptr, float fColor = COLOR_NONE)
	//{
	//	if (m_vcAbleParticleArray.empty()) return;

	//	auto it = m_vcAbleParticleArray.end() - 1;
	//	(*it)->Enable(pos, fColor);
	//	m_vcAbleParticleArray.pop_back();
	//}

	void EffectOn(int num, CGameObject * pObj, XMFLOAT3 * pos = nullptr, XMFLOAT3 * vel = nullptr, XMFLOAT3 * acc = nullptr, float fDamage = 0.f, float fColor = COLOR_NONE)
	{
		m_ppEffctsObjects[num]->Enable(pObj, pos, fColor);
		m_ppEffctsObjects[num]->SetDamage(fDamage);
		if (vel) m_ppEffctsObjects[num]->SetMoveVelocity(*vel);
		if (acc) m_ppEffctsObjects[num]->SetMoveAccel(*acc);
	}

	void EffectOn(EFFECT_ON_INFO & info)
	{
		EffectOn(info.iNum, info.m_pObject, &info.m_xmf3Pos, &info.m_xmf3Velocity, &info.m_xmfAccelate, info.fDamage, info.fColor);
	}
};

class CParticleShader : public CShader
{
// Normal
	ID3D11VertexShader       * m_pd3dVSSO;
	ID3D11GeometryShader     * m_pd3dGSSO;

private:
// Rain Particle
	ID3D11GeometryShader     * m_pd3dStreamRain;

	ID3D11VertexShader		 * m_pd3dVSRainDraw;
	ID3D11GeometryShader     * m_pd3dGSRainDraw;
	ID3D11PixelShader		 * m_pd3dPSRainDraw;

	CRainParticle			 * m_pRainParticle;

private:
	ID3D11DepthStencilState  * m_pd3dSODepthStencilState;
	ID3D11DepthStencilState  * m_pd3dDepthStencilState;

	ID3D11BlendState         * m_pd3dBlendState;

	ID3D11ShaderResourceView * m_pd3dRandomSRV;
	ID3D11SamplerState       * m_pd3dSamplerState;

//	ID3D11ShaderResourceView ** m_ppd3dParticleImageSRV;
//	int m_nImages;

private:

	CParticle    ** m_ppParticle;
	ID3D11Buffer *  m_pd3dcbGameInfo;

	//typedef CParticle* ParticleInfo;
	typedef pair<UINT, CParticle*> ParticleInfo;
	vector<CParticle*>   m_vcAbleParticleArray;
	vector<ParticleInfo> m_vcUsingParticleArray;

public :
	CParticleShader();
	virtual ~CParticleShader();

	void	CreateStates(ID3D11Device * pd3dDevice);
	virtual void CreateShader(ID3D11Device *pd3dDevice);

	virtual void BuildObjects(ID3D11Device *pd3dDevice, CMaterial * pMaterial);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera = nullptr);
	virtual void AnimateObjects(float fTimeElapsed);

	void SOSetState(ID3D11DeviceContext * pd3dDeviceContext);

	void RainDrawShader(ID3D11DeviceContext * pd3dDeviceContext);

	bool SetParticle(int index, CGameObject * pObj, XMFLOAT3 * pos = nullptr)
	{ 
		if (m_ppParticle[index]->IsAble() ) return false;
		return m_ppParticle[index]->Enable(pObj);
	}
	CParticle * GetParticle(int index) { return m_ppParticle[index]; }

	void ParticleOn(CGameObject * pObj = nullptr, XMFLOAT3 * pos = nullptr, float fColor = COLOR_NONE)
	{
		if (m_vcAbleParticleArray.empty()) return;

		auto it = m_vcAbleParticleArray.end() - 1;
		(*it)->Enable(pObj, pos, fColor);
		m_vcAbleParticleArray.pop_back();
	}

	void ParticleOn(int num, CGameObject * pObj = nullptr, XMFLOAT3 * pos = nullptr, XMFLOAT3 * vel = nullptr, XMFLOAT3 * acc = nullptr, float fDamage = 0.f, float fColor = COLOR_NONE)
	{
		m_ppParticle[num]->Enable(pObj, pos, fColor);
		m_ppParticle[num]->SetDamage(fDamage);
		if (vel) m_ppParticle[num]->SetMoveVelocity(*vel);
		if (acc) m_ppParticle[num]->SetMoveAccel(*acc);
	}

	void ParticleOn(EFFECT_ON_INFO & info)
	{
		ParticleOn(info.iNum, info.m_pObject, &info.m_xmf3Pos, &info.m_xmf3Velocity, &info.m_xmfAccelate, info.fDamage, info.fColor);
	}
};