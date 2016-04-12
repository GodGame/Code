#include "stdafx.h"
#include "MyInline.h"
#include "Weapon.h"
#include "Player.h"

CItem::CItem(int nMesh) : CStaticObject(nMesh)
{
	m_bObstacle = false;

	m_pMaster = nullptr;
	m_bThrowing = false;

	m_fFriction = 15.f;
}

CItem::~CItem()
{
	if (m_pMaster) m_pMaster->Release();
}

void CItem::GetGameMessage(CEntity * byEntity, eMessage eMSG, void * extra)
{
	switch(eMSG)
	{
	case eMessage::MSG_CULL_IN:
		m_bActive = true;
		return;
	case eMessage::MSG_CULL_OUT:
		m_bActive = false;
		return;
	case eMessage::MSG_COLLIDED :
		Collide(byEntity);
		return;
	}
}

bool CItem::IsVisible(CCamera * pCamera)
{
	OnPrepareRender();

	if (pCamera)
	{
		AABB bcBoundingCube = m_bcMeshBoundingCube;
		bcBoundingCube.Update(m_xmf44World);
		if (pCamera) m_bActive = pCamera->IsInFrustum(&bcBoundingCube);
	}
	return (m_bActive && m_pMaster == nullptr);
}

void CItem::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera, XMFLOAT4X4 * pmtxParentWorld)
{
	CGameObject::UpdateSubResources(pd3dDeviceContext, uRenderState, pCamera, pmtxParentWorld);
	CEntity::_ResetActive(uRenderState);

	if (m_pMaster)
	{
		CGameObject::_SetMaterialAndTexture(pd3dDeviceContext);
	}

	m_ppMeshes[0]->Render(pd3dDeviceContext, uRenderState);
}

void CItem::Update(float fFrameTime)
{
	const float fGravity = 30.f;
	const float fMaxSpeed = 80.f;

	XMFLOAT3 xmfPos = GetPosition();
#if 0
	XMVECTOR xmvVelocity = XMLoadFloat3(&m_xmf3ExternalPower);
	
	xmvVelocity = XMLoadFloat3(&xmfPos) + XMVector3Normalize(xmvVelocity) * (fMaxSpeed * fFrameTime);
	xmvVelocity -= XMVectorSet(0, fGravity * fFrameTime, 0, 0);
	XMStoreFloat3(&xmfPos, xmvVelocity);

	
	XMVECTOR xmvExternalPowerLength = XMVector3Length(xmvVelocity * fFrameTime);
	float fExternalPowerLength = 0.f;
	XMStoreFloat(&fExternalPowerLength, xmvExternalPowerLength);

	if (fExternalPowerLength < m_fFriction * fFrameTime)
	{
		m_xmf3ExternalPower = { 0, 0, 0 };
	}
	else
	{
		//cout << "Power : " << m_xmf3ExternalPower << endl;
		xmvVelocity = xmvVelocity + -xmvVelocity * XMVectorReplicate(m_fFriction * fFrameTime);
		XMStoreFloat3(&m_xmf3ExternalPower, xmvVelocity);
		//xmvVelocity = XMVector3Normalize(xmvVelocity);// *fMaxSpeed;
	}
	xmvVelocity = XMLoadFloat3(&xmfPos) + xmvVelocity - XMVectorSet(0, fGravity * fFrameTime, 0, 0);
	XMStoreFloat3(&xmfPos, xmvVelocity);

#endif
	xmfPos.y -= fGravity * fFrameTime;
	float fHeight = MAPMgr.GetHeight(xmfPos.x, xmfPos.z, (int(xmfPos.z) % 2)) + 1.f;
	if (xmfPos.y < fHeight)
	{
		xmfPos.y = fHeight;
		m_bThrowing = false;

		UpdateBoundingBox();
		QUADMgr.InsertStaticEntity(this);

		m_xmf3ExternalPower = { 0, 0, 0 };
	}
	//cout << "Throw : " << xmfPos << endl;
	SetPosition(xmfPos);
}

void CItem::Animate(float fTimeElapsed)
{
	if (m_bThrowing) Update(fTimeElapsed);
}

void CItem::Collide(CEntity * byEntity)
{
	if (m_bThrowing || m_pMaster) return;
	CInGamePlayer * pObj = dynamic_cast<CInGamePlayer*>(byEntity);
	if (nullptr == pObj || pObj->GetChildObject()) return;
	//cout << "Collide";

	SetMaster(pObj);
}

void CItem::SetMaster(CGameObject * pObj)
{
	if (m_pMaster) m_pMaster->Release();

	m_pMaster = pObj;
	// 오류나면 빼자. 부모 객체는 AddRedf 없애는 것이 나을 수도..
	m_pMaster->AddRef();
	m_pMaster->SetChild(this);
}

void CItem::ResetMaster()
{
	if (m_pMaster)
	{
		SetPosition(m_pMaster->GetPosition());
		m_pMaster->Release();
		m_pMaster = nullptr;

		AllocPositionAndEntityQuadTree();
	}
}

void CItem::ResetMaster(XMFLOAT3 & ThrowVelocity)
{
	if (m_pMaster)
	{
		//XMFLOAT3 xmfPos = m_pMaster->GetPosition();
		//xmfPos.y += 2.f;


		//SetPosition(xmfPos);
		m_pMaster->Release();
		m_pMaster = nullptr;

		AllocPositionAndEntityQuadTree(ThrowVelocity);
	}
}

void CItem::InheritByPlayer(const XMFLOAT3 & xmfRelativePos)
{
	// 포지션 변경하기전에 딜리트하자.
	QUADMgr.DeleteStaticEntity(this);

	SetPosition(xmfRelativePos);
	Rotate(-90, 0, 0);
	m_bThrowing = false;
}

void CItem::AllocPositionAndEntityQuadTree()
{
	m_bThrowing = true;
	Rotate(90, 0, 0);
	CMapManager & mapMgr = MAPMgr;

	XMFLOAT3 xmfPos;
	xmfPos.x = rand() % int(mapMgr.GetWidth());
	xmfPos.z = rand() % int(mapMgr.GetLength());
	xmfPos.y = 200.f; // mapMgr.GetHeight(xmfPos.x, xmfPos.z, int(xmfPos.z) % 2);
	SetPosition(xmfPos);
}

void CItem::AllocPositionAndEntityQuadTree(XMFLOAT3 & xmfVelocity)
{
	m_bThrowing = true;
	Rotate(90, 0, 0);
	SetPosition(xmfVelocity);
	//SetExternalPower(xmfVelocity);

	UpdateBoundingBox();
	QUADMgr.InsertStaticEntity(this);
}

void CItem::AllocPositionAndEntityQuadTree(float fx, float fy, float fz)
{
	m_bThrowing = true;
	Rotate(90, 0, 0);

	XMFLOAT3 xmfPos;
	xmfPos.x = fx;
	xmfPos.y = fy;
	xmfPos.z = fz;
	SetPosition(xmfPos);

	UpdateBoundingBox();
	QUADMgr.InsertStaticEntity(this);
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
