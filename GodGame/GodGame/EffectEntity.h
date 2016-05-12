#pragma once
#ifndef __EFFECT_H
#define __EFFECT_H

#include "Object.h"

enum EFFECT_TYPE
{
	EFFECT_NONE = 0,
	EFFECT_ABSORB,
	EFFECT_CASTING,
	EFFECT_FIREBALL,

	EFFECT_CIRCLE_AREA,
	EFFECT_ICE_BOLT,
	EFFECT_ETC
};

class CEffect : public CEntity
{
protected:
	CGameObject * m_pMaster;
	MoveVelocity m_velocity;

	float m_fDurability;
	float m_fDamage;

	bool m_bReserveDelete : 1;
	bool m_bEnable        : 1;
	bool m_bTerminal      : 1;
	bool m_bSubordinate   : 1;
	bool m_bMove          : 1;
	bool m_bUseAccel      : 1;

public:
	float GetDamage() { return m_fDamage; }
	void SetDamage(float dmg) { m_fDamage = dmg; }
		
	void SetMaster(CGameObject * pMaster) { m_pMaster = pMaster; }
	CGameObject * GetMaster() { return m_pMaster; }

	void SetDurabilityTime(float fTime) { m_fDurability = fTime; }
	float GetDurabilityTime()	const { return m_fDurability; }

	void SetMoveVelocity(const XMFLOAT3 & vel) { m_velocity.xmf3Velocity = vel; }
	void SetMoveAccel(const XMFLOAT3 & acc) { m_velocity.xmf3Accelate = acc; }

	bool IsUsing() { return m_bEnable; }
	bool IsSubordinative() { return m_bSubordinate; }

	bool MoveUpdate(const float & fGameTime, const float & fTimeElapsed, XMFLOAT3 & xmf3Pos);
	void SetMoveVelocity(MoveVelocity & move, XMFLOAT3 * InitPos);

protected:
	UINT m_nStartVertex;
	UINT m_nVertexStrides;
	UINT m_nVertexOffsets;

	ID3D11ShaderResourceView * m_pd3dSRVImagesArrays;
	ID3D11Buffer * m_pd3dDrawVertexBuffer;

public:
	CEffect();
	virtual ~CEffect();

	void SetShaderResourceView(ID3D11ShaderResourceView * pSRV)
	{
		if (m_pd3dSRVImagesArrays) m_pd3dSRVImagesArrays->Release();
		m_pd3dSRVImagesArrays = pSRV;
		m_pd3dSRVImagesArrays->AddRef();
	}
	inline void UpdateShaderResourceArray(ID3D11DeviceContext * pd3dDeviceContext)
	{
		pd3dDeviceContext->PSSetShaderResources(TX_SLOT_TEXTURE_ARRAY, 1, &m_pd3dSRVImagesArrays);
	}
	inline void UpdateShaderResource(ID3D11DeviceContext * pd3dDeviceContext)
	{
		pd3dDeviceContext->PSSetShaderResources(0, 1, &m_pd3dSRVImagesArrays);
	}
	virtual void UpdateBoundingBox() = 0;
	virtual void NextEffectOn() {}

	virtual void GetGameMessage(CEntity * byEntity, eMessage eMSG, void * extra = nullptr);
	virtual void SendGameMessage(CEntity * toEntity, eMessage eMSG, void * extra = nullptr);

	virtual bool Enable(CPlayer * pPlayer, XMFLOAT3 * pos = nullptr, int fColorNum = COLOR_NONE) { return false; }
	virtual bool Disable() { return false; }
	virtual void Collide();
};
/////////////////////////////////////////////////////////////////////////////////////////////////////
struct TX_ANIMATION_VERTEX
{
	XMFLOAT2	xmf2FrameRatePercent;
	XMFLOAT2	xmf2FrameTotalSize;
};

class CTxAnimationObject : public CEffect
{
protected:
	CB_TX_ANIMATION_INFO m_cbInfo;
	CTxAnimationObject * m_pNextEffect;
	ID3D11Buffer * m_pd3dCSBuffer;
	bool	 m_bUseAnimation : 1;
	bool     m_bUseLoop : 1;
	//	float	 m_fTimeStartUseAnimation;

public:
	CTxAnimationObject();
	virtual ~CTxAnimationObject();

	virtual void Initialize(ID3D11Device *pd3dDevice);
	void CreateBuffers(ID3D11Device * pd3dDevice, XMFLOAT2 & xmf2ObjSize, XMFLOAT2 & xmf2FrameTotalSize, XMFLOAT2 & xmf2FrameSize, UINT dwFrameNum, float dwFramePerTime);
	/* 1 : info 구조체, 2 : 한 프레임 너비, 3 : 한 프레임 높이, 4 : 프레임 총 개수, 5 : 프레임 당 시간  */
	void CalculateCSInfoTime(TX_ANIMATION_VERTEX & vertex, XMFLOAT2 & xmf2ObjSize, XMFLOAT2 & xmf2FrameSize, UINT dwFrameNum, float dwFramePerTime);
	void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext);

public:
	void UseAnimation() { m_bUseAnimation = true; }
	virtual XMFLOAT3 GetPosition() const { return m_cbInfo.m_xmf3Pos; }
	virtual void UpdateBoundingBox()
	{
		m_bcMeshBoundingCube.Update(m_cbInfo.m_xmf3Pos, m_uSize);
	}

	virtual bool Enable(CGameObject * pObj, XMFLOAT3 * pos = nullptr, int fColorNum = COLOR_NONE);
	virtual bool Disable();
	virtual void NextEffectOn();
	bool IsTermainal() { return (m_pNextEffect) ? m_pNextEffect->IsTermainal() : !m_bEnable; }

public:
	void OnPrepare(ID3D11DeviceContext * pd3dDeviceContext) { UpdateShaderResource(pd3dDeviceContext); }
	void Animate(float fTimeElapsed);
	void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld = nullptr);
};

class CLightBomb : public CTxAnimationObject
{
public:
	virtual void Initialize(ID3D11Device *pd3dDevice);
};

class CCircleMagic : public CTxAnimationObject
{
public:
	virtual void Initialize(ID3D11Device *pd3dDevice);
};

class CIceSpear : public CTxAnimationObject
{
public:
	virtual void Initialize(ID3D11Device *pd3dDevice);
};

class CIceBolt : public CTxAnimationObject
{
public:
	virtual void Initialize(ID3D11Device *pd3dDevice);
};

class CElectricBolt : public CTxAnimationObject
{
public:
	virtual void Initialize(ID3D11Device *pd3dDevice);
};

class CElementSpike : public CTxAnimationObject
{
public:
	virtual void Initialize(ID3D11Device *pd3dDevice);
};

class CStaticFlame : public CTxAnimationObject
{
public:
	virtual void Initialize(ID3D11Device *pd3dDevice);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////



struct PARTICLE_INFO
{
	XMFLOAT3 m_xmf3Pos;
	XMFLOAT3 m_xmf3Velocity;
	XMFLOAT2 m_xmf2Size;
	float	 m_fAge;
	float	 m_uType;
};

struct EFFECT_ON_INFO
{
	CGameObject * m_pObject;
	XMFLOAT3 m_xmf3Pos;
	XMFLOAT3 m_xmf3Velocity;
	XMFLOAT3 m_xmfAccelate;
	bool bUseUpdateVelocity;
	float fDamage;
	float fColor;
	EFFECT_TYPE eEffect;
};

class CParticle : public CEffect
{
protected:
	CB_PARTICLE m_cbParticle;

	bool m_bInitilize : 1;
	//UINT m_bExtra     : 28;
	ID3D11Buffer * m_pd3dInitialVertexBuffer;
	ID3D11Buffer * m_pd3dStreamOutVertexBuffer;

	ID3D11Buffer * m_pd3dCSParticleBuffer;

protected:
	CParticle   * m_pcNextParticle;

public:
	CParticle();
	virtual ~CParticle();

	CB_PARTICLE * GetCBParticle() { return &m_cbParticle; }
	void Update(float fTimeElapsed);
	void LifeUpdate(const float & fGameTime, const float & fTimeElapsed);

	virtual void Initialize(ID3D11Device *pd3dDevice);
	void SetParticle(CB_PARTICLE & info, MoveVelocity & Velocity, float fDurability, UINT uMaxParticle);
	void CreateParticleBuffer(ID3D11Device *pd3dDevice, PARTICLE_INFO & pcInfo, UINT nMaxNum);

	void StreamOut(ID3D11DeviceContext *pd3dDeviceContext);
	void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld = nullptr);
	void OnPrepare(ID3D11DeviceContext * pd3dDeviceContext) { UpdateShaderResourceArray(pd3dDeviceContext); }
	void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext);

public:

	virtual XMFLOAT3 GetPosition() const { return m_cbParticle.m_vParticleEmitPos; }
	virtual void UpdateBoundingBox()
	{
		m_bcMeshBoundingCube.Update(m_cbParticle.m_vParticleEmitPos, m_uSize);
		//cout << "BB : " << m_bcMeshBoundingCube << endl;
	}

public:
	void SetLifeTime(float fLifeTime) { m_cbParticle.m_fLifeTime = fLifeTime; }
	void SetEmitPosition(XMFLOAT3 & pos) { m_cbParticle.m_vParticleEmitPos = pos; }
	void SetEmitDirection(XMFLOAT3 & dir) { m_cbParticle.m_vParticleVelocity = dir; }
	void SetAccelation(XMFLOAT3 & accel) { m_cbParticle.m_vAccel = accel; }
	void SetParticleSize(float fSize) { m_cbParticle.m_fMaxSize = fSize; }

	virtual bool Enable(CGameObject * pObj, XMFLOAT3 * pos = nullptr, int fColorNum = COLOR_NONE);
	virtual bool Disable();
	virtual void NextEffectOn();
	//void NextParticleOn();

	bool IsTermainal() { return (m_pcNextParticle) ? m_pcNextParticle->IsTermainal() : !m_bEnable; }
};

class CSmokeBoomParticle : public CParticle
{
	static const UINT m_nMaxParticlenum = 200;

public:
	CSmokeBoomParticle() {}
	virtual ~CSmokeBoomParticle() {}

	virtual void Initialize(ID3D11Device *pd3dDevice);
};

class CFireBallParticle : public CParticle
{
	static const UINT m_nMaxParticlenum = 500;

public:
	CFireBallParticle() {}
	virtual ~CFireBallParticle() {}

	virtual void Initialize(ID3D11Device *pd3dDevice);
};

class CRainParticle : public CParticle
{
	static const UINT m_nMaxParticlenum = 4000;

public:
	CRainParticle() {}
	virtual ~CRainParticle() {}

	//void StreamOut(ID3D11DeviceContext *pd3dDeviceContext);
	virtual void Initialize(ID3D11Device *pd3dDevice);
};
#endif