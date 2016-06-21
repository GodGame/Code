#include "stdafx.h"
#include "MyInline.h"
#include "Object.h"
#include "Shader.h"
//////////////////////////////////////////////////////////////////////////

CEntity::CEntity()
{
	m_uSize       = 0;
	m_bVisible    = true;
	m_bActive     = true;
	m_bUseCollide = false;
	m_bObstacle   = true;
	m_bDetailCollide = false;

	ZeroMemory(&m_bcMeshBoundingCube, sizeof(m_bcMeshBoundingCube));
}

void CEntity::GetGameMessage(CEntity * byEntity, eMessage eMSG, void * extra)
{
	switch (eMSG)
	{
	case eMessage::MSG_CULL_IN:
		m_bVisible = true;
		return;
	case eMessage::MSG_CULL_OUT:
		m_bVisible = false;
		return;
	case eMessage::MSG_COLLIDE:
		return;
	case eMessage::MSG_COLLIDED:
		return;
	}
}

void CEntity::SendGameMessage(CEntity * toEntity, eMessage eMSG, void * extra)
{
	switch (eMSG)
	{
	case eMessage::MSG_NORMAL:
		return;
		// 반대로 메세지 전송하도록 하자
	case eMessage::MSG_COLLIDE:
		toEntity->GetGameMessage(this, MSG_COLLIDED);
		return;
	case eMessage::MSG_COLLIDED:
		toEntity->GetGameMessage(this, MSG_COLLIDE);
		return;
	}
}

void CEntity::MessageEntToEnt(CEntity * byEntity, CEntity * toEntity, eMessage eMSG, void * extra)
{
	byEntity->SendGameMessage(toEntity, eMSG);
	toEntity->GetGameMessage(byEntity, eMSG);
}

#pragma region GameObject
CGameObject::CGameObject(int nMeshes)
{
	Chae::XMFloat4x4Identity(&m_xmf44World);

	m_bUseInheritAutoRender = true;

	m_nMeshes  = nMeshes;
	m_ppMeshes = nullptr;

	if (m_nMeshes > 0)
		m_ppMeshes = new CMesh*[m_nMeshes];

	for (int i = 0; i < m_nMeshes; i++)
		m_ppMeshes[i] = nullptr;

	m_bcMeshBoundingCube = AABB();

	m_nReferences = 0;
	m_pMaterial   = nullptr;
	m_pTexture    = nullptr;

	m_pChild      = nullptr;
	m_pSibling    = nullptr;
	//m_pParent     = nullptr;
}

CGameObject::~CGameObject()
{
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Release();
			m_ppMeshes[i] = nullptr;
		}
		delete[] m_ppMeshes;
	}
	if (m_pMaterial) m_pMaterial->Release();
	if (m_pTexture) m_pTexture->Release();

	// child, sibling은 ReleaseRelationShip을 이용해 외부에서 지워준다.
}

void CGameObject::_SetMaterialAndTexture(ID3D11DeviceContext * pd3dDeviceContext)
{
	if (m_pMaterial) CIlluminatedShader::UpdateShaderVariable(pd3dDeviceContext, &m_pMaterial->m_Material);
	if (m_pTexture) m_pTexture->UpdateShaderVariable(pd3dDeviceContext);
}

void CGameObject::AddRef() { m_nReferences++; }

void CGameObject::Release()
{
	//if (m_nReferences > 0) m_nReferences--;
	if (--m_nReferences < 1)
	{
		ReleaseRelationShip();

		delete this;
	}
}

void CGameObject::ReleaseRelationShip()
{
	// 루트가 아니라면, 자식은 모두 지우고 형제는 부모에게 넘겨주고 삭제한다.
	if (m_pChild)   m_pChild->Release();
	//if (m_pParent)	SuccessSibling();
	if (m_pSibling) m_pSibling->Release();
}

void CGameObject::SetTexture(CTexture* const pTexture, bool beforeRelease)
{
	if (beforeRelease && m_pTexture) m_pTexture->Release();
	m_pTexture = pTexture;
	if (m_pTexture) m_pTexture->AddRef();
}

void CGameObject::UpdateSubResources(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld)
{
	XMFLOAT4X4 xmf44Result;

	if (false == m_bUseInheritAutoRender) 
		CShader::UpdateShaderVariable(pd3dDeviceContext, m_xmf44World);
	else if (pmtxParentWorld)
	{
		Chae::XMFloat4x4Mul(&xmf44Result, &m_xmf44World, pmtxParentWorld);

		if (m_pChild)   m_pChild->Render(pd3dDeviceContext, uRenderState, pCamera, &xmf44Result);
		if (m_pSibling) m_pSibling->Render(pd3dDeviceContext, uRenderState, pCamera, pmtxParentWorld);

		CShader::UpdateShaderVariable(pd3dDeviceContext, xmf44Result);
	}
	else
	{
		if (m_pChild)   m_pChild->Render(pd3dDeviceContext, uRenderState, pCamera, &m_xmf44World);
		if (m_pSibling) m_pSibling->Render(pd3dDeviceContext, uRenderState, pCamera, pmtxParentWorld);

		CShader::UpdateShaderVariable(pd3dDeviceContext, m_xmf44World);
	}
}

void CGameObject::UpdateBoundingBox()
{
	m_bcMeshBoundingCube.Update(m_xmf44World, &m_ppMeshes[0]->GetBoundingCube());
}

void CGameObject::SetMesh(CMesh* const pMesh, int nIndex)
{
	if (m_ppMeshes)
	{
		if (m_ppMeshes[nIndex]) m_ppMeshes[nIndex]->Release();
		m_ppMeshes[nIndex] = pMesh;
		if (pMesh) pMesh->AddRef();
	}

	if (pMesh)
	{
		AABB bcBoundingCube = pMesh->GetBoundingCube();
		m_bcMeshBoundingCube.Union(&bcBoundingCube);
		//m_bcMeshBoundingCube.Update(m_xmf44World, nullptr);

		XMVECTOR xmvMax = XMLoadFloat3(&m_bcMeshBoundingCube.m_xv3Maximum);
		XMVECTOR xmvMin = XMLoadFloat3(&m_bcMeshBoundingCube.m_xv3Minimum);

		xmvMax = xmvMax - xmvMin;
		xmvMax = XMVector3Length(xmvMax);

		float fSize;
		XMStoreFloat(&fSize, xmvMax);
		m_uSize = (UINT)(fSize * 0.25f);
	}
	else
	{
		m_bcMeshBoundingCube.m_xv3Maximum = { FLT_MIN, FLT_MIN, FLT_MIN };
		m_bcMeshBoundingCube.m_xv3Minimum = { FLT_MAX, FLT_MAX, FLT_MAX };
	}
}

void CGameObject::Animate(float fTimeElapsed)
{
	if (m_pChild) m_pChild->Animate(fTimeElapsed);
	if (m_pSibling) m_pSibling->Animate(fTimeElapsed);
}

bool CGameObject::IsVisible(CCamera * pCamera)
{
	OnPrepareRender();

	if (pCamera)
	{
		AABB bcBoundingCube = m_bcMeshBoundingCube;
		bcBoundingCube.Update(m_xmf44World);
		if (pCamera) m_bVisible = pCamera->IsInFrustum(&bcBoundingCube);
	}
	return(m_bVisible);
}

void CGameObject::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld)
{
	if (false == m_bActive) return;
	if (m_bVisible || uRenderState & RS_SHADOWMAP || (pmtxParentWorld && m_bUseInheritAutoRender))
	{
		CGameObject::UpdateSubResources(pd3dDeviceContext, uRenderState, pCamera, pmtxParentWorld);
		CGameObject::_SetMaterialAndTexture(pd3dDeviceContext);

		for (int i = 0; i < m_nMeshes; i++)
		{
#ifdef _QUAD_TREE
			m_ppMeshes[i]->Render(pd3dDeviceContext, uRenderState);
			if (false == (uRenderState & DRAW_AND_ACTIVE))
				m_bVisible = false;
#else
			bool bIsVisible = true;
			if (pCamera)
			{
				AABB bcBoundingCube = m_ppMeshes[i]->GetBoundingCube();
				bcBoundingCube.Update(m_xmf44World);
				bIsVisible = pCamera->IsInFrustum(&bcBoundingCube);
			}
			if (bIsVisible)
				m_ppMeshes[i]->Render(pd3dDeviceContext, uRenderState);
#endif
		}
	}
}
 
void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf44World._41 = x;
	m_xmf44World._42 = y;
	m_xmf44World._43 = z;
}
void CGameObject::SetPosition(const XMFLOAT3& xv3Position)
{
//	memcpy(&m_xmf44World._41, &xv3Position, sizeof(XMFLOAT3));
	m_xmf44World._41 = xv3Position.x;
	m_xmf44World._42 = xv3Position.y;
	m_xmf44World._43 = xv3Position.z;
}
void CGameObject::SetPosition(const XMVECTOR * xv3Position)
{
	XMFLOAT3 xmf3Pos;
	XMStoreFloat3(&xmf3Pos, *xv3Position);
	SetPosition(xmf3Pos);
}
#ifdef PICKING
void CGameObject::GenerateRayForPicking(XMFLOAT3 *pxv3PickPosition, XMFLOAT4X4 *pxmtxWorld, XMFLOAT4X4 *pxmtxView, XMFLOAT3 *pxv3PickRayPosition, XMFLOAT3 *pxv3PickRayDirection)
{
	//pxv3PickPosition: 카메라 좌표계의 점(화면 좌표계에서 마우스를 클릭한 점을 역변환한 점)
	//pxmtxWorld: 월드 변환 행렬, pxmtxView: 카메라 변환 행렬
	//pxv3PickRayPosition: 픽킹 광선의 시작점, pxv3PickRayDirection: 픽킹 광선 벡터
	/*객체의 월드 변환 행렬이 주어지면 객체의 월드 변환 행렬과 카메라 변환 행렬을 곱하고 역행렬을 구한다. 이것은 카메라 변환 행렬의 역행렬과 객체의 월드 변환 행렬의 역행렬의 곱과 같다. 객체의 월드 변환 행렬이 주어지지 않으면 카메라 변환 행렬의 역행렬을 구한다. 객체의 월드 변환 행렬이 주어지면 모델 좌표계의 픽킹 광선을 구하고 그렇지 않으면 월드 좌표계의 픽킹 광선을 구한다.*/
	XMFLOAT4X4 xmtxInverse;
	XMFLOAT4X4 xmtxWorldView = *pxmtxView;
	if (pxmtxWorld) XMFLOAT4X4Multiply(&xmtxWorldView, pxmtxWorld, pxmtxView);
	XMFLOAT4X4Inverse(&xmtxInverse, nullptr, &xmtxWorldView);
	XMFLOAT3 xv3CameraOrigin(0.0f, 0.0f, 0.0f);
	/*카메라 좌표계의 원점 (0, 0, 0)을 위에서 구한 역행렬로 변환한다. 변환의 결과는 카메라 좌표계의 원점에 대응되는 모델 좌표계의 점 또는 월드 좌표계의 점이다.*/
	xv3ec3TransformCoord(pxv3PickRayPosition, &xv3CameraOrigin, &xmtxInverse);
	/*카메라 좌표계의 점을 위에서 구한 역행렬로 변환한다. 변환의 결과는 마우스를 클릭한 점에 대응되는 모델 좌표계의 점 또는 월드 좌표계의 점이다.*/
	xv3ec3TransformCoord(pxv3PickRayDirection, pxv3PickPosition, &xmtxInverse);
	//픽킹 광선의 방향 벡터를 구한다.
	*pxv3PickRayDirection = *pxv3PickRayDirection - *pxv3PickRayPosition;
}

int CGameObject::PickObjectByRayIntersection(XMFLOAT3 *pxv3PickPosition, XMFLOAT4X4 *pxmtxView, MESHINTERSECTINFO *pd3dxIntersectInfo)
{
	//pxv3PickPosition: 카메라 좌표계의 점(화면 좌표계에서 마우스를 클릭한 점을 역변환한 점)
	//pxmtxView: 카메라 변환 행렬
	XMFLOAT3 xv3PickRayPosition, xv3PickRayDirection;
	int nIntersected = 0;
	//활성화된 객체에 대하여 메쉬가 있으면 픽킹 광선을 구하고 객체의 메쉬와 충돌 검사를 한다.
	if (m_bVisible && m_ppMeshes)
	{
		//객체의 모델 좌표계의 픽킹 광선을 구한다.
		GenerateRayForPicking(pxv3PickPosition, &m_xmf44World, pxmtxView, &xv3PickRayPosition, &xv3PickRayDirection);
		/*모델 좌표계의 픽킹 광선과 메쉬의 충돌을 검사한다. 픽킹 광선과 메쉬의 삼각형들은 여러 번 충돌할 수 있다. 검사의 결과는 충돌된 횟수이다.*/
		for (int i = 0; i < m_nMeshes; i++)
		{
			nIntersected = m_ppMeshes[i]->CheckRayIntersection(&xv3PickRayPosition, &xv3PickRayDirection, pd3dxIntersectInfo);
			if (nIntersected > 0) break;
		}
	}
	return(nIntersected);
}
#endif

void CGameObject::SetMaterial(CMaterial *pMaterial)
{
	if (m_pMaterial) m_pMaterial->Release();
	m_pMaterial = pMaterial;
	if (m_pMaterial) m_pMaterial->AddRef();
}

void CGameObject::ChangeChild(CGameObject * pObject)
{
	if (m_pChild)
	{
		CGameObject * pSiblingOfChild = m_pChild->GetSiblingObject();
		
		if (pSiblingOfChild)
			pObject->SetSibling(pSiblingOfChild);

		m_pChild->Release();
	}
	m_pChild = pObject;
	m_pChild->AddRef();
}

void CGameObject::SetChild(CGameObject * pObject)
{
	CGameObject * pBeforeChild = nullptr;
	pBeforeChild = m_pChild;

	m_pChild = pObject;
	if (m_pChild)	m_pChild->AddRef();

	if (pBeforeChild) 
	{
		if (m_pChild) m_pChild->SetSibling(pBeforeChild);
		pBeforeChild->Release();
	}
}

void CGameObject::SetSibling(CGameObject * pObject)
{
	CGameObject * pBeforeSibling = nullptr;
	pBeforeSibling = m_pSibling;

	if (pBeforeSibling)
	{
		pBeforeSibling->SetSibling(m_pSibling);
	}
	else
	{
		m_pSibling = pObject;
		m_pSibling->AddRef();
	}
}

XMFLOAT3 CGameObject::GetLookAt() const 
{
	//게임 객체를 로컬 z-축 벡터를 반환한다.
	XMFLOAT3 xv3LookAt(m_xmf44World._31, m_xmf44World._32, m_xmf44World._33);
	//Chae::XMFloat3Normalize(&xv3LookAt);
	return(xv3LookAt);
}
XMFLOAT3 CGameObject::GetUp() const
{
	//게임 객체를 로컬 y-축 벡터를 반환한다.
	XMFLOAT3 xv3Up(m_xmf44World._21, m_xmf44World._22, m_xmf44World._23);
	//Chae::XMFloat3Normalize(&xv3Up);
	return(xv3Up);
}
XMFLOAT3 CGameObject::GetRight() const
{
	//게임 객체를 로컬 x-축 벡터를 반환한다.
	XMFLOAT3 xv3Right(m_xmf44World._11, m_xmf44World._12, m_xmf44World._13);
	//Chae::XMFloat3Normalize(&xv3Right);
	return(xv3Right);
}

void CGameObject::MoveStrafe(float fDistance)
{
	//게임 객체를 로컬 x-축 방향으로 이동한다.
	XMVECTOR xmv3Position = XMLoadFloat3(&GetPosition());
	XMVECTOR xv3Right = XMLoadFloat3(&GetRight());
	xmv3Position += fDistance * xv3Right;

	CGameObject::SetPosition(&xmv3Position);
}
void CGameObject::MoveUp(float fDistance)
{
	//게임 객체를 로컬 y-축 방향으로 이동한다.
	XMVECTOR xv3Position = XMLoadFloat3(&GetPosition());
	XMVECTOR xv3Up = XMLoadFloat3(&GetUp());
	xv3Position += fDistance * xv3Up;
	CGameObject::SetPosition(&xv3Position);
}
void CGameObject::MoveForward(float fDistance)
{
	//게임 객체를 로컬 z-축 방향으로 이동한다.
	XMVECTOR xv3Position = XMLoadFloat3(&GetPosition());
	XMVECTOR xv3LookAt = XMLoadFloat3(&GetLookAt());
	xv3Position += fDistance * xv3LookAt;
	CGameObject::SetPosition(&xv3Position);
}
void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	//게임 객체를 주어진 각도로 회전한다.
	XMMATRIX mtxWorld = XMLoadFloat4x4(&m_xmf44World);
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	//XMFLOAT4X4RotationYawPitchRoll(&mtxRotate, (float)D3DXToRadian(fYaw), (float)D3DXToRadian(fPitch), (float)D3DXToRadian(fRoll));
	mtxWorld = XMMatrixMultiply(mtxRotate, mtxWorld);
	XMStoreFloat4x4(&m_xmf44World, mtxWorld);
}
void CGameObject::Rotate(XMFLOAT3 *pxv3Axis, float fAngle)
{
	//게임 객체를 주어진 회전축을 중심으로 회전한다.
	XMMATRIX mtxWorld = XMLoadFloat4x4(&m_xmf44World);
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxv3Axis), XMConvertToRadians(fAngle));

	mtxWorld = XMMatrixMultiply(mtxRotate, mtxWorld);
	XMStoreFloat4x4(&m_xmf44World, mtxWorld);
}

XMFLOAT3 CGameObject::GetPosition() const
{
	return(XMFLOAT3(m_xmf44World._41, m_xmf44World._42, m_xmf44World._43));
}

#pragma endregion


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CStaticObject::CStaticObject(int nMeshes) : CGameObject(nMeshes)
{
	m_xmf3ExternalPower = { 0, 0, 0 };
	m_fFriction = 0.f;
}

CStaticObject::~CStaticObject()
{
}

CDynamicObject::CDynamicObject(int nMeshes) : CGameObject(nMeshes)
{
	m_xv3Velocity = { 0, 0, 0 };
}


CAnimatedObject::CAnimatedObject(int nMeshes) : CDynamicObject(nMeshes)
{
	m_vcfAnimationCycleTime.resize(nMeshes, 1.0f);
	m_vcfFramePerTime.resize(nMeshes, 1.0f);

	m_wdAnimateState = 0;
}

CAnimatedObject::~CAnimatedObject()
{
}

void CAnimatedObject::ChangeAnimationState(WORD wd, bool bReserveIdle, WORD * pNextStateArray, int nNum)
{
	if (m_wdAnimateState != wd)
	{
		m_wdAnimateState = wd;
		static_cast<CAnimatedMesh*>(m_ppMeshes[m_wdAnimateState])->ResetIndex();

		m_bReserveBackIdle = bReserveIdle;

		if (pNextStateArray)
		{
			m_vcNextAnimState.clear();

			for (int i = 0; i < nNum; ++i)
				m_vcNextAnimState.push_back(*(pNextStateArray + nNum - (i + 1)));
		}

	}
}

void CAnimatedObject::SetAnimationCycleTime(WORD wdAnimNum, float fCycleTime)
{
	m_vcfAnimationCycleTime[wdAnimNum] = fCycleTime;

	if (m_ppMeshes[wdAnimNum])
	{
		m_vcfFramePerTime[wdAnimNum] =
			static_cast<ANI_MESH*>(m_ppMeshes[wdAnimNum])->SetOneCycleTime(m_vcfAnimationCycleTime[wdAnimNum]);
	}
}

void CAnimatedObject::UpdateFramePerTime()
{
	for (int i = 0; i < m_nMeshes; ++i)
	{
		if (m_ppMeshes[i])
		{
			m_vcfFramePerTime[i] =
				static_cast<ANI_MESH*>(m_ppMeshes[i])->SetOneCycleTime(m_vcfAnimationCycleTime[i]);
		}
	}
}

void CAnimatedObject::SetMesh(CMesh * pMesh, int nIndex)
{
	CLoadAnimatedMeshByADFile * pAnimMesh = dynamic_cast<CLoadAnimatedMeshByADFile*>(pMesh);
	CMesh * pTempMesh = pMesh;
	
	if (pAnimMesh) 
	{
		CAnimatedMesh * pCopyMesh = new CAnimatedMesh(*pAnimMesh);
		pTempMesh = pCopyMesh;
	}

	CGameObject::SetMesh(pTempMesh, nIndex);
	
	m_vcfFramePerTime[nIndex] =
		static_cast<ANI_MESH*>(m_ppMeshes[nIndex])->SetOneCycleTime(m_vcfAnimationCycleTime[nIndex]);
}

void CAnimatedObject::UpdateBoundingBox()
{
	m_bcMeshBoundingCube.Update(m_xmf44World, &m_ppMeshes[0]->GetBoundingCube());// m_wdAnimateState]->GetBoundingCube());
}

void CAnimatedObject::Animate(float fTimeElapsed)
{
	ANI_MESH * pMesh = static_cast<ANI_MESH*>(m_ppMeshes[m_wdAnimateState]);
	pMesh->SetFramePerTime(m_vcfFramePerTime[m_wdAnimateState]);
	pMesh->Animate(fTimeElapsed);
	
	if (pMesh->IsEndAnimation())
	{
		if (!m_vcNextAnimState.empty())
		{
			ChangeAnimationState(*(m_vcNextAnimState.end() - 1), m_bReserveBackIdle, nullptr, 0);
			m_vcNextAnimState.pop_back();
		}
		else if (m_bReserveBackIdle)
		{
			ChangeAnimationState(eANI_IDLE, false, nullptr, 0);
			m_bReserveBackIdle = false;
		}
	}
}

void CAnimatedObject::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld)
{
	if (false == m_bActive) return;

	if (m_ppMeshes && m_bVisible)
	{
		CGameObject::UpdateSubResources(pd3dDeviceContext, uRenderState, pCamera, pmtxParentWorld);
		//객체의 재질(상수버퍼)을 쉐이더 변수에 설정(연결)한다.
		if (m_pMaterial) CIlluminatedShader::UpdateShaderVariable(pd3dDeviceContext, &m_pMaterial->m_Material);
		//객체의 텍스쳐를 쉐이더 변수에 설정(연결)한다.
		if (m_pTexture) m_pTexture->UpdateShaderVariable(pd3dDeviceContext);
#ifdef _QUAD_TREE
		m_ppMeshes[m_wdAnimateState]->Render(pd3dDeviceContext, uRenderState);
		if (!(uRenderState & DRAW_AND_ACTIVE))
			m_bVisible = false;
#else
		bool bIsVisible = true;
		if (pCamera)
		{
			AABB bcBoundingCube = m_ppMeshes[i]->GetBoundingCube();
			bcBoundingCube.Update(m_xmf44World);
			bIsVisible = pCamera->IsInFrustum(&bcBoundingCube);
		}
		if (bIsVisible)
			m_ppMeshes[i]->Render(pd3dDeviceContext, uRenderState);
#endif
	}
}


////////////////////////////////////////////////////////////////////////////////////////
CUIObject::CUIObject(int nMeshes, UIInfo info) : CGameObject(nMeshes) 
{
	if (info.m_msgUI != MSG_UI_NONE)
	{
		SetCollide(true);
	}
	
	m_info = info;
}

bool CUIObject::CollisionCheck(XMFLOAT3 & pos)
{
	return CollisionCheck(POINT{ (LONG)pos.x, (LONG)pos.y });
}

bool CUIObject::CollisionCheck(POINT & pt)
{
	if (!m_bUseCollide)		  return false;

	RECT & rect = m_info.m_rect;

	if (pt.x > rect.right)  return false;
	if (pt.x < rect.left)   return false;
	if (pt.y > rect.top)    return false;
	if (pt.y < rect.bottom) return false;

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CRotatingObject::CRotatingObject(int nMeshes) : CGameObject(nMeshes)
{
	m_fRotationSpeed = 15.0f;
}
CRotatingObject::~CRotatingObject()
{
}

void CRotatingObject::Animate(float fTimeElapsed)
{
	CGameObject::Animate(fTimeElapsed);
	CGameObject::Rotate(&m_xv3RotationAxis, m_fRotationSpeed * fTimeElapsed);
}

CRevolvingObject::CRevolvingObject(int nMeshes) : CGameObject(nMeshes)
{
	m_xv3RevolutionAxis = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_fRevolutionSpeed = 0.0f;
}

CRevolvingObject::~CRevolvingObject()
{
}

void CRevolvingObject::Animate(float fTimeElapsed)
{
	CGameObject::Animate(fTimeElapsed);
	//CGameObject::Rotate(&m_xv3RevolutionAxis, XMConvertToRadians(m_fRevolutionSpeed * fTimeElapsed));
	//공전을 나타내기 위해 회전 행렬을 오른쪽에 곱한다.
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xv3RevolutionAxis), (float)XMConvertToRadians(m_fRevolutionSpeed * fTimeElapsed));
	XMMATRIX mtxWorld = XMLoadFloat4x4(&m_xmf44World);
	mtxWorld = mtxWorld * mtxRotate;

	XMStoreFloat4x4(&m_xmf44World, mtxWorld);
}

CHeightMapTerrain::CHeightMapTerrain(ID3D11Device *pd3dDevice, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xv3Scale) : CGameObject(0)
{
	MAPMgr.Build(pFileName, nWidth, nLength, nBlockWidth, nBlockLength, xv3Scale);

	/*지형 객체는 격자 메쉬들의 배열로 만들 것이다. nBlockWidth, nBlockLength는 격자 메쉬 하나의 가로, 세로 크기이다. cxQuadsPerBlock, czQuadsPerBlock은 격자 메쉬의 가로 방향과 세로 방향 사각형의 개수이다.*/
	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;
#ifdef TS_TERRAIN
	CTerrainPartMesh *pHeightMapGridMesh = nullptr;
	for (float zStart = 0; zStart < m_xv3Scale.z; zStart++)
	{
		for (float xStart = 0; xStart < m_xv3Scale.x; xStart++)
		{
			//지형의 일부분을 나타내는 격자 메쉬의 시작 위치이다.
			//지형의 일부분을 나타내는 격자 메쉬를 생성하여 지형 메쉬에 저장한다.
			pHeightMapGridMesh = new CTerrainPartMesh(pd3dDevice, xStart, zStart, nBlockWidth, nBlockLength, xv3Scale, m_pHeightMap);
			SetMesh(pHeightMapGridMesh, xStart + (zStart*m_xv3Scale.x));
		}
	}
#else
	//지형에서 가로 방향, 세로 방향으로 격자 메쉬가 몇 개가 있는 가를 나타낸다.
	int cxBlocks = (nWidth - 1) / cxQuadsPerBlock;
	int czBlocks = (nLength - 1) / czQuadsPerBlock;
	//지형 전체를 표현하기 위한 격자 메쉬의 개수이다.
	m_nMeshes = cxBlocks * czBlocks;
	//지형 전체를 표현하기 위한 격자 메쉬에 대한 포인터 배열을 생성한다.
	m_ppMeshes = new CMesh*[m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)m_ppMeshes[i] = nullptr;

	CHeightMap * heightMap = MAPMgr.GetHeightMap();
	CHeightMapGridMesh *pHeightMapGridMesh = nullptr;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			//지형의 일부분을 나타내는 격자 메쉬의 시작 위치이다.
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			//지형의 일부분을 나타내는 격자 메쉬를 생성하여 지형 메쉬에 저장한다.
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, xStart, zStart, nBlockWidth, nBlockLength, xv3Scale, heightMap);
			SetMesh(pHeightMapGridMesh, x + (z*cxBlocks));
		}
	}
#endif
	MAPMgr.SetMaxHeight(m_bcMeshBoundingCube.m_xv3Maximum.y);
	MAPMgr.SetMinHeight(m_bcMeshBoundingCube.m_xv3Minimum.y);

	Chae::XMFloat4x4Identity(&m_xmf44World);
}

CHeightMapTerrain::~CHeightMapTerrain()
{
//	if (m_pHeightMap) delete m_pHeightMap;
}

CWaterTerrain::CWaterTerrain(ID3D11Device * pd3dDevice, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xv3Scale)
{
	/*지형 객체는 격자 메쉬들의 배열로 만들 것이다. nBlockWidth, nBlockLength는 격자 메쉬 하나의 가로, 세로 크기이다. cxQuadsPerBlock, czQuadsPerBlock은 격자 메쉬의 가로 방향과 세로 방향 사각형의 개수이다.*/
	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;
	
	int cxBlocks = (nWidth - 1) / cxQuadsPerBlock;
	int czBlocks = (nLength - 1) / czQuadsPerBlock;
	//지형 전체를 표현하기 위한 격자 메쉬의 개수이다.
	m_nMeshes = cxBlocks * czBlocks;
	//지형 전체를 표현하기 위한 격자 메쉬에 대한 포인터 배열을 생성한다.
	m_ppMeshes = new CMesh*[m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)m_ppMeshes[i] = nullptr;

	CHeightMap * heightMap = MAPMgr.GetHeightMap();
	CWaterGridMesh *pHeightMapGridMesh = nullptr;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			//지형의 일부분을 나타내는 격자 메쉬의 시작 위치이다.
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			//지형의 일부분을 나타내는 격자 메쉬를 생성하여 지형 메쉬에 저장한다.
			pHeightMapGridMesh = new CWaterGridMesh(pd3dDevice, xStart, zStart, nBlockWidth, nBlockLength, xv3Scale, heightMap);
			SetMesh(pHeightMapGridMesh, x + (z*cxBlocks));
		}
	}
	Chae::XMFloat4x4Identity(&m_xmf44World);
}

CWaterTerrain::~CWaterTerrain()
{
}


#define SKYBOX_CUBE
CSkyBox::CSkyBox(ID3D11Device *pd3dDevice, UINT uImageNum) : CGameObject(1)
{
	CSkyBoxMesh *pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, uImageNum, 20.0f, 20.0f, 20.0f);
	SetMesh(pSkyBoxMesh, 0);
}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld)
{
#ifdef SKYBOX_CUBE
	XMFLOAT3 xv3CameraPos = pCamera->GetPosition();
	SetPosition(xv3CameraPos.x, xv3CameraPos.y, xv3CameraPos.z);
	CShader::UpdateShaderVariable(pd3dDeviceContext, m_xmf44World);
	//m_pTexture->UpdateShaderVariable(pd3dDeviceContext);

	if (m_ppMeshes && m_ppMeshes[0])
		m_ppMeshes[0]->Render(pd3dDeviceContext, uRenderState);

#else
	//스카이 박스 객체의 위치를 카메라의 위치로 변경한다.
	XMFLOAT3 xv3CameraPos = pCamera->GetPosition();
	SetPosition(xv3CameraPos.x, xv3CameraPos.y, xv3CameraPos.z);
	CShader::UpdateShaderVariable(pd3dDeviceContext, &m_xmf44World);

	//스카이 박스 메쉬(6개의 사각형)를 렌더링한다.
	if (m_ppMeshes && m_ppMeshes[0]) m_ppMeshes[0]->Render(pd3dDeviceContext);
#endif
}

CBillboardObject::CBillboardObject(XMFLOAT3 pos, UINT fID, XMFLOAT2 xmf2Size) : CGameObject(1)
{
	m_xv4InstanceData = XMFLOAT4(pos.x, pos.y, pos.z, (float)fID);
	m_xv2Size = xmf2Size;
	SetSize((m_xv2Size.x + m_xv2Size.y) * 0.5);
	SetPosition(pos);
}

void CBillboardObject::UpdateInstanceData()
{
	const XMFLOAT3 pos = GetPosition();
	memcpy(&m_xv4InstanceData, &pos, sizeof(XMFLOAT3));
}

bool CBillboardObject::IsVisible(CCamera *pCamera)
{
	bool bIsVisible = m_bVisible;

	if (pCamera)
	{
		m_bcMeshBoundingCube.Update(m_xmf44World, &m_ppMeshes[0]->GetBoundingCube());
		bIsVisible = pCamera->IsInFrustum(&m_bcMeshBoundingCube);
	}

	XMFLOAT3 xmfPos = GetPosition();
	memcpy(&m_xv4InstanceData, &xmfPos, sizeof(XMFLOAT3));
	return(bIsVisible);
}

//////////////////////////////////////
CAbsorbMarble::CAbsorbMarble() : CBillboardObject()
{
	m_bObstacle = false;
}

CAbsorbMarble::CAbsorbMarble(XMFLOAT3 pos, UINT fID, XMFLOAT2 xmf2Size) : CBillboardObject(pos, fID, xmf2Size)
{
	//Initialize();
}

CAbsorbMarble::~CAbsorbMarble()
{
}

void CAbsorbMarble::Initialize()
{
	m_bObstacle = false;
	SetCollide(true);

	m_bAbsorb          = false;
	m_fAbsorbTime      = 0.0f;
	m_fSpeed           = 0.0f;
	m_pTargetObject    = nullptr;

	SetSize(miAbsorbSize);

	m_xvRandomVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

void CAbsorbMarble::Revive()
{
	CMapManager * pTerrain = nullptr;
	XMFLOAT3 xmf3Pos = MAPMgr.GetRandPos();
	xmf3Pos.y += 10.f;

	Initialize();
	SetPosition(xmf3Pos);

	SetCollide(true);
}

void CAbsorbMarble::SetTarget(CGameObject * pGameObject)
{
	if (false == m_bAbsorb)
	{
		SetSize(10);
		SetCollide(false);

		m_pTargetObject = pGameObject;
		m_bAbsorb = true;

		XMVECTOR xmvTarget = XMLoadFloat3(&m_pTargetObject->GetPosition());
		XMVECTOR xvPos = XMLoadFloat3(&CBillboardObject::GetPosition());
		XMVECTOR xmvFromTarget = xvPos - xmvTarget;
		xmvFromTarget = XMVector3Normalize(xmvFromTarget);
		XMStoreFloat3(&m_xvRandomVelocity, xmvFromTarget);

		m_xvRandomVelocity.y = abs(m_xvRandomVelocity.y);
		m_fSpeed = rand() % 10 + 10.0f;
	}
}

void CAbsorbMarble::Animate(float fTimeElapsed)
{
	if (m_bAbsorb)
	{
		m_fAbsorbTime		+= fTimeElapsed;
		XMFLOAT3 xmfPos      = m_pTargetObject->GetPosition();
		xmfPos.y += 10;
		XMVECTOR xmvTarget   = XMLoadFloat3(&xmfPos);
		XMVECTOR xvPos       = XMLoadFloat3(&CBillboardObject::GetPosition());
		XMVECTOR xmvToTarget = xmvTarget - xvPos;
		xmvToTarget          = XMVector3Normalize(xmvToTarget) * 2.0f;

		XMVECTOR xvSpeed     = XMVectorReplicate(m_fSpeed);
		XMVECTOR xvRandom    = XMLoadFloat3(&m_xvRandomVelocity);

		xvPos = (0.5f * xmvToTarget * m_fAbsorbTime * m_fAbsorbTime) + (xvRandom * m_fAbsorbTime) + xvPos;
		XMStoreFloat3(&xmfPos, xvPos);
		SetPosition(xmfPos);
		CBillboardObject::UpdateInstanceData();

		XMVECTOR lengthSq = XMVector3LengthSq(xmvTarget - xvPos);
		float flengthSq;
		XMStoreFloat(&flengthSq, lengthSq);

		if (flengthSq < 60.0f && m_fAbsorbTime > 0.3f)
		{
			GetGameMessage(this, eMessage::MSG_COLLIDE);
		}
	}
}

void CAbsorbMarble::GetGameMessage(CEntity * byObj, eMessage eMSG, void * extra)
{
	CInGamePlayer * pPlayer = nullptr;

	switch (eMSG)
	{
	case eMessage::MSG_CULL_IN:
		m_bVisible = true;
		return;
	case eMessage::MSG_CULL_OUT:
		m_bVisible = false;
		return;
	case eMessage::MSG_COLLIDE:
		m_pTargetObject->GetGameMessage(this, eMessage::MSG_PLAYER_SOUL, &GetInstanceData());
		Revive();
		//EVENTMgr.InsertDelayMessage(1.0f, eMessage::MSG_OBJECT_RENEW, CGameEventMgr::MSG_TYPE_OBJECT, this);
		return;
	case eMessage::MSG_COLLIDED:
		if (pPlayer = dynamic_cast<CInGamePlayer*>(byObj))
			SetTarget(pPlayer);
		return;

	//case eMessage::MSG_QUAD_DELETE:

	//	return;
	case eMessage::MSG_OBJECT_RENEW:
		CBillboardObject::UpdateInstanceData();
		SetCollide(true);
		QUADMgr.InsertStaticEntity(this);
		return;
	case eMessage::MSG_NORMAL:
		return;
	}
}

bool CAbsorbMarble::IsVisible(CCamera *pCamera)
{
	bool bIsVisible = m_bVisible;

	if (pCamera)
		bIsVisible = CBillboardObject::IsVisible(pCamera);
	else if (bIsVisible || m_bAbsorb)
		bIsVisible = m_bVisible = true;

	return(bIsVisible);
}

