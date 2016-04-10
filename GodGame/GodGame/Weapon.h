#pragma once
#ifndef __WEAPON_H
#define __WEAPON_H

#include "Object.h"

class CItem : public CStaticObject
{
protected:
	CGameObject * m_pMaster;

public:
	CItem(int nMesh);
	virtual ~CItem();

	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld = nullptr);

public:
	void SetMaster(CGameObject * pObj);
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
