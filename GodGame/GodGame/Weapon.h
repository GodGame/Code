#pragma once
#ifndef __WEAPON_H
#define __WEAPON_H

#include "Object.h"

class CItem : public CStaticObject
{
protected:
	CGameObject * m_pMaster;
	bool m_bThrowing : 1;

public:
	CItem(int nMesh);
	virtual ~CItem();

public:
	//virtual void SendGameMessage(CEntity * toEntity, eMessage eMSG, void * extra = nullptr);
	virtual void GetGameMessage(CEntity * byEntity, eMessage eMSG, void * extra = nullptr);
	virtual bool IsVisible(CCamera *pCamera = nullptr);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld = nullptr);

	virtual void Update(float fFrameTime);
	virtual void Animate(float fTimeElapsed);

public:
	void Collide(CEntity * byEntity);

	void SetMaster(CGameObject * pObj);
	void ResetMaster();
	void ResetMaster(XMFLOAT3 & ThrowVelocity);

	void InheritByPlayer(const XMFLOAT3 & xmfRelativePos);
	void AllocPositionAndEntityQuadTree();
	void AllocPositionAndEntityQuadTree(float fx, float fy, float fz);
	void AllocPositionAndEntityQuadTree(XMFLOAT3 & xmfPos);

	CGameObject * GetMaster() { return m_pMaster; }
};

class CEquipment : public CItem
{
protected:
	WORD m_wdDurability;

public:
	CEquipment(int nMesh);
	virtual ~CEquipment();
};

class CWeapon : public CEquipment
{
protected:
	UINT m_uDamage : 8;

public:
	CWeapon(int nMesh);
	virtual ~CWeapon();
};

class CEffect;
class CTxAnimationObject;
class CParticle;
class Element;

class CStaff : public CWeapon
{
protected:
	UINT m_uCost : 8;

	CEffect * m_pHasEffect;

public:
	CStaff(int nMesh);
	virtual ~CStaff();
};

#endif
