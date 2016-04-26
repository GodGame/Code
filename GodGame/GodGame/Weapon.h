#pragma once
#ifndef __WEAPON_H
#define __WEAPON_H

#define WEAPON_LEVEL_MAX 3

#include "Object.h"
#include "GameInfo.h"
class CInGamePlayer;
class CItem : public CStaticObject
{
protected:
	CInGamePlayer * m_pMaster;
	bool m_bThrowing : 1;
	UINT m_iNumber;

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

	void SetMaster(CInGamePlayer * pObj);
	void ResetMaster();
	void ResetMaster(XMFLOAT3 & ThrowVelocity);

	void InheritByPlayer(const XMFLOAT3 & xmfRelativePos);
	void AllocPositionAndEntityQuadTree();
	void AllocPositionAndEntityQuadTree(float fx, float fy, float fz);
	void AllocPositionAndEntityQuadTree(XMFLOAT3 & xmfPos);

	CInGamePlayer * GetMaster() { return m_pMaster; }
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
//class Element;

class CStaff : public CWeapon
{
protected:
	UINT mCost  : 8;
	UINT mLevel : 2;

	ELEMENT mElement;
	CEffect * m_pHasEffect;
	
public:
	CStaff(int nMesh);
	virtual ~CStaff();

	DWORD     GetCost()      { return mCost; }
	WORD      GetLevel()     { return mLevel; }
	ELEMENT   GetElement()   { return mElement; }
	CEffect * GetEffect()    { return m_pHasEffect; }

	void SetEffect(CEffect * pEffect) { m_pHasEffect = pEffect; }
	void SetLevel(int lv) { mLevel = lv; }

public:
	void BuildObject(ELEMENT element, DWORD cost, DWORD level);
	void BuildObject(CStaff & staff);

};

#endif
