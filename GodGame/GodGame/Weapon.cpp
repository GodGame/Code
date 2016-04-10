#include "stdafx.h"
#include "Weapon.h"



CItem::CItem(int nMesh) : CStaticObject(nMesh)
{
	m_pMaster = nullptr;
}

CItem::~CItem()
{
	if (m_pMaster) m_pMaster->Release();
}

void CItem::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera, XMFLOAT4X4 * pmtxParentWorld)
{
	CGameObject::UpdateSubResources(pd3dDeviceContext, uRenderState, pCamera, pmtxParentWorld);

	m_ppMeshes[0]->Render(pd3dDeviceContext, uRenderState);

	if (false == (uRenderState & DRAW_AND_ACTIVE))
		m_bActive = false;
}

void CItem::SetMaster(CGameObject * pObj)
{
	if (m_pMaster) m_pMaster->Release();
	m_pMaster = pObj;
	m_pMaster->AddRef();
}

///////////////////////////////////////////////////////////////////
CEquipment::CEquipment(int nMesh) : CItem(nMesh)
{
	m_wdDurability = 0;
}

CEquipment::~CEquipment()
{
}

////////////////////////////////////////////////////////////////
CWeapon::CWeapon(int nMesh) : CEquipment(nMesh)
{
}

CWeapon::~CWeapon()
{
}

//////////////////////////////////////////////////////////////////
CStaff::CStaff(int nMesh) : CWeapon(nMesh)
{
	m_pHasEffect = nullptr;
}

CStaff::~CStaff()
{
}
