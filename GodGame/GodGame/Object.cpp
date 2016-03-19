#include "stdafx.h"
#include "MyInline.h"
#include "Object.h"
#include "Shader.h"
//////////////////////////////////////////////////////////////////////////
#pragma region GameObject
CGameObject::CGameObject(int nMeshes)
{
	Chae::XMFloat4x4Identity(&m_xmf44World);

	m_nMeshes  = nMeshes;
	m_ppMeshes = nullptr;
	if (m_nMeshes > 0)
		m_ppMeshes = new CMesh*[m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)m_ppMeshes[i] = nullptr;

	m_bcMeshBoundingCube = AABB();

	m_bActive     = true;
	m_bUseCollide = false;

	m_nReferences = 0;
	m_pMaterial   = nullptr;
	m_pTexture    = nullptr;

	m_pChild      = nullptr;
	m_pSibling    = nullptr;
	m_pParent     = nullptr;
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

void CGameObject::SuccessSibling()
{
	if (m_pParent->m_nReferences < 1) return;

	if (m_pSibling) m_pParent->SetChild(m_pSibling);
	m_pSibling->Release();
}

void CGameObject::ReleaseRelationShip()
{
	// 루트가 아니라면, 자식은 모두 지우고 형제는 부모에게 넘겨주고 삭제한다.
	if (m_pChild)   m_pChild->Release();
	if (m_pParent)	SuccessSibling();
	if (m_pSibling) m_pSibling->Release();
}

void CGameObject::SetTexture(CTexture *pTexture, bool beforeRelease)
{
	if (beforeRelease && m_pTexture) m_pTexture->Release();
	m_pTexture = pTexture;
	if (m_pTexture) m_pTexture->AddRef();
}

void CGameObject::UpdateBoundingBox()
{
	m_bcMeshBoundingCube.Update(m_xmf44World, &m_ppMeshes[0]->GetBoundingCube());//&m_ppMeshes[0]->GetBoundingCube());
}

void CGameObject::SetMesh(CMesh *pMesh, int nIndex)
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
		m_uSize = (UINT)(fSize / 3.0f);
	}
}

void CGameObject::Animate(float fTimeElapsed)
{
}

bool CGameObject::IsVisible(CCamera * pCamera)
{
	OnPrepareRender();

	if (pCamera)
	{
		AABB bcBoundingCube = m_bcMeshBoundingCube;
		bcBoundingCube.Update(m_xmf44World);
		if (pCamera) m_bActive = pCamera->IsInFrustum(&bcBoundingCube);
	}
	return(m_bActive);
}

void CGameObject::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	CShader::UpdateShaderVariable(pd3dDeviceContext, m_xmf44World);
	//객체의 재질(상수버퍼)을 쉐이더 변수에 설정(연결)한다.
	if (m_pMaterial) CIlluminatedShader::UpdateShaderVariable(pd3dDeviceContext, &m_pMaterial->m_Material);
	//객체의 텍스쳐를 쉐이더 변수에 설정(연결)한다.
	if (m_pTexture) m_pTexture->UpdateShaderVariable(pd3dDeviceContext);

	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
#ifdef _QUAD_TREE
			if (m_bActive)
			{
				m_ppMeshes[i]->Render(pd3dDeviceContext, uRenderState);
				if(!(uRenderState & DRAW_AND_ACTIVE)) 
					m_bActive = false;
			}
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
void CGameObject::SetPosition(XMFLOAT3& xv3Position)
{
	m_xmf44World._41 = xv3Position.x;
	m_xmf44World._42 = xv3Position.y;
	m_xmf44World._43 = xv3Position.z;
}
void CGameObject::SetPosition(XMVECTOR * xv3Position)
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
	if (m_bActive && m_ppMeshes)
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

	m_pSibling = pObject; 
	if (m_pSibling) m_pSibling->AddRef();	
	if (pBeforeSibling) pBeforeSibling->Release();
}

void CGameObject::SetParent(CGameObject * pObject)
{
	m_pParent = pObject;
	// 단일방향으로 지우기 위해서 부모는 AddRef 호출을 안한다.
	if (pObject && m_pSibling) m_pSibling->SetParent(pObject);
	//if (pBeforeParent) pBeforeParent->Release();
}

XMFLOAT3 CGameObject::GetLookAt()
{
	//게임 객체를 로컬 z-축 벡터를 반환한다.
	XMFLOAT3 xv3LookAt(m_xmf44World._31, m_xmf44World._32, m_xmf44World._33);
	//Chae::XMFloat3Normalize(&xv3LookAt);
	return(xv3LookAt);
}
XMFLOAT3 CGameObject::GetUp()
{
	//게임 객체를 로컬 y-축 벡터를 반환한다.
	XMFLOAT3 xv3Up(m_xmf44World._21, m_xmf44World._22, m_xmf44World._23);
	//Chae::XMFloat3Normalize(&xv3Up);
	return(xv3Up);
}
XMFLOAT3 CGameObject::GetRight()
{
	//게임 객체를 로컬 x-축 벡터를 반환한다.
	XMFLOAT3 xv3Right(m_xmf44World._11, m_xmf44World._12, m_xmf44World._13);
	//Chae::XMFloat3Normalize(&xv3Right);
	return(xv3Right);
}

void CGameObject::GetGameMessage(CGameObject * byObj, eMessage eMSG, void * extra)
{
	switch (eMSG)
	{
	case eMessage::MSG_CULL_IN:
		m_bActive = true;
		return;
	case eMessage::MSG_CULL_OUT:
		m_bActive = false;
		return;
	case eMessage::MSG_COLLIDE:
		return;
	case eMessage::MSG_COLLIDED:
		return;
	case eMessage::MSG_NORMAL:
		return;
	}
}

void CGameObject::SendGameMessage(CGameObject * toObj, eMessage eMSG, void * extra)
{
	switch (eMSG)
	{
	case eMessage::MSG_NORMAL:
		return;
	// 반대로 메세지 전송하도록 하자
	case eMessage::MSG_COLLIDE:
		toObj->GetGameMessage(this, MSG_COLLIDED);
		return;
	case eMessage::MSG_COLLIDED:
		toObj->GetGameMessage(this, MSG_COLLIDE);
		return;
	}
}

void CGameObject::MessageObjToObj(CGameObject * byObj, CGameObject * toObj, eMessage eMSG, void * extra)
{
	byObj->SendGameMessage(toObj, eMSG);
	toObj->GetGameMessage(byObj, eMSG);
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

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf44World._41, m_xmf44World._42, m_xmf44World._43));
}

#pragma endregion
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CStaticObject::CStaticObject(int nMeshes) : CGameObject(nMeshes)
{
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
	m_wdAnimateState = 0;
}

void CAnimatedObject::Animate(float fTimeElapsed)
{
	static_cast<ANI_MESH*>(m_ppMeshes[m_wdAnimateState])->Animate(fTimeElapsed);
}

void CAnimatedObject::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	CShader::UpdateShaderVariable(pd3dDeviceContext, m_xmf44World);
	//객체의 재질(상수버퍼)을 쉐이더 변수에 설정(연결)한다.
	if (m_pMaterial) CIlluminatedShader::UpdateShaderVariable(pd3dDeviceContext, &m_pMaterial->m_Material);
	//객체의 텍스쳐를 쉐이더 변수에 설정(연결)한다.
	if (m_pTexture) m_pTexture->UpdateShaderVariable(pd3dDeviceContext);

	if (m_ppMeshes)
	{
#ifdef _QUAD_TREE
		if (m_bActive)
		{
			m_ppMeshes[m_wdAnimateState]->Render(pd3dDeviceContext, uRenderState);
			if (!(uRenderState & DRAW_AND_ACTIVE))
				m_bActive = false;
		}
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
	CGameObject::Rotate(&m_xv3RevolutionAxis, XMConvertToRadians(m_fRevolutionSpeed * fTimeElapsed));
	//공전을 나타내기 위해 회전 행렬을 오른쪽에 곱한다.
	//XMFLOAT4X4 mtxRotate;
	//XMFLOAT4X4RotationAxis(&mtxRotate, &m_xv3RevolutionAxis, (float)D3DXToRadian(m_fRevolutionSpeed * fTimeElapsed));
	//m_xmf44World = m_xmf44World * mtxRotate;
}

CHeightMap::CHeightMap(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3& xv3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xv3Scale = xv3Scale;

	BYTE *pHeightMapImage = new BYTE[m_nWidth * m_nLength];

	//파일을 열고 읽는다. 높이 맵 이미지는 파일 헤더가 없는 RAW 이미지이다.
	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, nullptr);
	DWORD dwBytesRead;
	::ReadFile(hFile, pHeightMapImage, (m_nWidth * m_nLength), &dwBytesRead, nullptr);
	::CloseHandle(hFile);

	/*이미지의 y-축과 지형의 z-축이 방향이 반대이므로 이미지를 상하대칭 시켜 저장한다. 그러면 <그림 7>과 같이 이미지의 좌표축과 지형의 좌표축의 방향이 일치하게 된다.*/
	m_pHeightMapImage = new BYTE[m_nWidth * m_nLength];
	for (int y = 0; y < m_nLength; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			m_pHeightMapImage[x + ((m_nLength - 1 - y)*m_nWidth)] = pHeightMapImage[x + (y*m_nWidth)];
		}
	}

	if (pHeightMapImage) delete[] pHeightMapImage;
}

CHeightMap::~CHeightMap()
{
	if (m_pHeightMapImage) delete[] m_pHeightMapImage;
	//m_pHeightMapImage = nullptr;
}

XMFLOAT3& CHeightMap::GetHeightMapNormal(int x, int z)
{
	//지형의 x-좌표와 z-좌표가 지형(높이 맵)의 범위를 벗어나면 지형의 법선 벡터는 y-축 방향 벡터이다.
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength)) return(XMFLOAT3(0.0f, 1.0f, 0.0f));

	/*높이 맵에서 (x, z) 좌표의 픽셀 값과 인접한 두 개의 점 (x+1, z), (z, z+1)에 대한 픽셀 값을 사용하여 법선 벡터를 계산한다.*/
	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -(signed)m_nWidth;
	//(x, z), (x+1, z), (z, z+1)의 지형의 높이 값을 구한다.
	float y1 = (float)m_pHeightMapImage[nHeightMapIndex] * m_xv3Scale.y;
	float y2 = (float)m_pHeightMapImage[nHeightMapIndex + xHeightMapAdd] * m_xv3Scale.y;
	float y3 = (float)m_pHeightMapImage[nHeightMapIndex + zHeightMapAdd] * m_xv3Scale.y;

	//vEdge1은 (0, y3, m_vScale.z) - (0, y1, 0) 벡터이다.
	XMFLOAT3 vEdge1 = XMFLOAT3(0.0f, y3 - y1, m_xv3Scale.z);
	//vEdge2는 (m_vScale.x, y2, 0) - (0, y1, 0) 벡터이다.
	XMFLOAT3 vEdge2 = XMFLOAT3(m_xv3Scale.x, y2 - y1, 0.0f);
	//법선 벡터는 vEdge1과 vEdge2의 외적을 정규화하면 된다.
	XMFLOAT3 vNormal;
	XMVECTOR xmvNormal, xmvEdge1, xmvEdge2;
	xmvEdge1 = XMLoadFloat3(&vEdge1); xmvEdge2 = XMLoadFloat3(&vEdge2);

	xmvNormal = XMVector3Cross(xmvEdge1, xmvEdge2);
	xmvNormal = XMVector3Normalize(xmvNormal);

	XMStoreFloat3(&vNormal, xmvNormal);
	return(vNormal);
}

float CHeightMap::GetHeight(float fx, float fz, bool bReverseQuad)
{
	//지형의 좌표 (fx, fz)에서 높이 맵의 좌표를 계산한다.
	fx = fx / m_xv3Scale.x;
	fz = fz / m_xv3Scale.z;
	//높이 맵의 x-좌표와 z-좌표가 높이 맵의 범위를 벗어나면 지형의 높이는 0이다.
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);
	//높이 맵의 좌표의 정수 부분과 소수 부분을 계산한다.
	int x = (int)fx, z = (int)fz;
	float fxPercent = fx - x, fzPercent = fz - z;

	float fTopLeft = m_pHeightMapImage[x + (z*m_nWidth)];
	float fTopRight = m_pHeightMapImage[(x + 1) + (z*m_nWidth)];
	float fBottomLeft = m_pHeightMapImage[x + ((z + 1)*m_nWidth)];
	float fBottomRight = m_pHeightMapImage[(x + 1) + ((z + 1)*m_nWidth)];

	if (bReverseQuad)
	{
		/*지형의 삼각형들이 오른쪽에서 왼쪽 방향으로 나열되는 경우이다. <그림 12>의 오른쪽은 (fzPercent < fxPercent)인 경우이다. 이 경우 TopLeft의 픽셀 값은 (fTopLeft = fTopRight + (fBottomLeft - fBottomRight))로 근사한다. <그림 12>의 왼쪽은 (fzPercent ≥ fxPercent)인 경우이다. 이 경우 BottomRight의 픽셀 값은 (fBottomRight = fBottomLeft + (fTopRight - fTopLeft))로 근사한다.*/
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		/*지형의 삼각형들이 왼쪽에서 오른쪽 방향으로 나열되는 경우이다. <그림 13>의 왼쪽은 (fzPercent < (1.0f - fxPercent))인 경우이다. 이 경우 TopRight의 픽셀 값은 (fTopRight = fTopLeft + (fBottomRight - fBottomLeft))로 근사한다. <그림 13>의 오른쪽은 (fzPercent ≥ (1.0f - fxPercent))인 경우이다. 이 경우 BottomLeft의 픽셀 값은 (fBottomLeft = fTopLeft + (fBottomRight - fTopRight))로 근사한다.*/
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
	//사각형의 네 점을 보간하여 높이(픽셀 값)를 계산한다.
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight);
}

CHeightMapTerrain::CHeightMapTerrain(ID3D11Device *pd3dDevice, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xv3Scale) : CGameObject(xv3Scale.x * xv3Scale.z)
{
	//지형에 사용할 높이 맵의 가로, 세로의 크기이다.
	m_nWidth = nWidth;
	m_nLength = nLength;

	/*지형 객체는 격자 메쉬들의 배열로 만들 것이다. nBlockWidth, nBlockLength는 격자 메쉬 하나의 가로, 세로 크기이다. cxQuadsPerBlock, czQuadsPerBlock은 격자 메쉬의 가로 방향과 세로 방향 사각형의 개수이다.*/
	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	//xv3Scale는 지형을 실제로 몇 배 확대할 것인가를 나타낸다.
	m_xv3Scale = xv3Scale;

	//지형에 사용할 높이 맵을 생성한다.
	m_pHeightMap = new CHeightMap(pFileName, nWidth, nLength, xv3Scale);

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
	int cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	int czBlocks = (m_nLength - 1) / czQuadsPerBlock;
	//지형 전체를 표현하기 위한 격자 메쉬의 개수이다.
	m_nMeshes = cxBlocks * czBlocks;
	//지형 전체를 표현하기 위한 격자 메쉬에 대한 포인터 배열을 생성한다.
	m_ppMeshes = new CMesh*[m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)m_ppMeshes[i] = nullptr;

	CHeightMapGridMesh *pHeightMapGridMesh = nullptr;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			//지형의 일부분을 나타내는 격자 메쉬의 시작 위치이다.
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			//지형의 일부분을 나타내는 격자 메쉬를 생성하여 지형 메쉬에 저장한다.
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, xStart, zStart, nBlockWidth, nBlockLength, xv3Scale, m_pHeightMap);
			SetMesh(pHeightMapGridMesh, x + (z*cxBlocks));
		}
	}
#endif

	Chae::XMFloat4x4Identity(&m_xmf44World);
}

CHeightMapTerrain::~CHeightMapTerrain()
{
	if (m_pHeightMap) delete m_pHeightMap;
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

void CSkyBox::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
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
	SetPosition(pos);
}

void CBillboardObject::UpdateInstanceData()
{
	XMFLOAT3 pos = GetPosition();
	memcpy(&m_xv4InstanceData, &pos, sizeof(XMFLOAT3));
}

bool CBillboardObject::IsVisible(CCamera *pCamera)
{
	bool bIsVisible = m_bActive;

	if (pCamera)
	{
		m_bcMeshBoundingCube.Update(m_xmf44World, &m_ppMeshes[0]->GetBoundingCube());
		bIsVisible = pCamera->IsInFrustum(&m_bcMeshBoundingCube);
	}

	XMFLOAT3 xmfPos = GetPosition();
	memcpy(&m_xv4InstanceData, &xmfPos, sizeof(XMFLOAT3));
	return(bIsVisible);
}
///////////////////////////////////////////////////////////////////////////////////////
CEffect::CEffect()
{
	m_nStartVertex        = 0;
	m_nVertexOffsets      = 0;
	m_nVertexStrides      = 0;

	m_fDurability         = 0;

	m_bEnable      = false;
	m_bMove        = false;
	m_bTerminal    = false;
	m_bSubordinate = false;

	m_pd3dSRVImagesArrays = nullptr;
	m_pd3dDrawVertexBuffer = nullptr;
}

CEffect::~CEffect()
{
	if (m_pd3dSRVImagesArrays) m_pd3dSRVImagesArrays->Release();
	if (m_pd3dDrawVertexBuffer) m_pd3dDrawVertexBuffer->Release();
}

void CEffect::MoveUpdate(const float & fGameTime, const float & fTimeElapsed, XMFLOAT3 & xmf3Pos)
{
	XMVECTOR xmvVelocity;
	XMVECTOR xmvPos;

	if (m_bUseAccel)
	{
		xmvVelocity = (m_velocity.fWeightSpeed * 0.5f * XMLoadFloat3(&m_velocity.xmf3Accelate) * fGameTime * fGameTime) +
			(m_velocity.fWeightSpeed * XMLoadFloat3(&m_velocity.xmf3Velocity) * fGameTime);
		xmvPos = xmvVelocity + XMLoadFloat3(&m_velocity.xmf3InitPos);
	}
	else
	{
		xmvVelocity = XMLoadFloat3(&m_velocity.xmf3Velocity) * fTimeElapsed * m_velocity.fWeightSpeed;
		xmvPos = xmvVelocity + XMLoadFloat3(&xmf3Pos);
	}
	XMStoreFloat3(&xmf3Pos, xmvPos);
}
void CEffect::SetMoveVelocity(MoveVelocity & move, XMFLOAT3 * InitPos)
{
	m_velocity = move;

	if (Chae::XMFloat3NorValue(m_velocity.xmf3Velocity, 0.0f)) m_bMove = true;
	if (Chae::XMFloat3NorValue(m_velocity.xmf3Accelate, 0.0f)) m_bUseAccel = true;

	if (InitPos) m_velocity.xmf3InitPos = *InitPos;
}
///////////////////////////////////////////////////////////////////////////////////////

CTxAnimationObject::CTxAnimationObject()
{
	ZeroMemory(&m_cbInfo, sizeof(m_cbInfo));
	m_pNextEffect = nullptr;
	m_pd3dCSBuffer = nullptr;

	m_bUseAnimation = false;
}

CTxAnimationObject::~CTxAnimationObject()
{
	if (m_pNextEffect) delete m_pNextEffect;
	if (m_pd3dCSBuffer) m_pd3dCSBuffer->Release();
}

void CTxAnimationObject::Initialize(ID3D11Device * pd3dDevice)
{
	m_cbInfo.m_nColorNum      = COLOR_WHITE;
	m_cbInfo.m_xmf3Pos        = { 0, 0, 0 };
	
	MoveVelocity move;
	move.xmf3Velocity         = { 0, 0, 0 };
	move.xmf3Accelate         = { 0, 0, 0 };
	move.fWeightSpeed         = 1.0f;
	SetMoveVelocity(move, &m_cbInfo.m_xmf3Pos);

	XMFLOAT2 xmf2Size		{ 50, 50 };
	XMFLOAT2 xmf2ImageSize	{ 1000, 200 };
	XMFLOAT2 xmf2FrameSize	{ 100, 100 };
	CTxAnimationObject::CreateBuffers(pd3dDevice, xmf2Size, xmf2ImageSize, xmf2FrameSize, 15, 0.1f);
}

void CTxAnimationObject::CreateBuffers(ID3D11Device * pd3dDevice, XMFLOAT2 & xmf2ObjSize, XMFLOAT2 & xmf2ImageSize, XMFLOAT2 & xmf2FrameSize, UINT dwFrameNum, float dwFramePerTime)
{
	TX_ANIMATION_VERTEX vertex;
	vertex.xmf2FrameTotalSize = xmf2ImageSize;

	CTxAnimationObject::CalculateCSInfoTime(vertex, xmf2ObjSize, xmf2FrameSize, dwFrameNum, dwFramePerTime);

	m_nVertexStrides        = sizeof(TX_ANIMATION_VERTEX);

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage     = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = m_nVertexStrides * 1;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;// | D3D11_BIND_STREAM_OUTPUT;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = &vertex;// pQuadPatchVertices;
	ASSERT_S(pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dDrawVertexBuffer));

	m_pd3dCSBuffer = ViewMgr.GetBuffer("cs_tx_animation");
	m_pd3dCSBuffer->AddRef();

}

void CTxAnimationObject::CalculateCSInfoTime(TX_ANIMATION_VERTEX & vertex, XMFLOAT2 & xmf2ObjSize, XMFLOAT2 & xmf2FrameSize, UINT dwFrameNum, float dwFramePerTime)
{
	UINT uWidth   = UINT(vertex.xmf2FrameTotalSize.x / xmf2FrameSize.x);
	UINT uHeight  = UINT(vertex.xmf2FrameTotalSize.y / xmf2FrameSize.y);

	vertex.xmf2FrameRatePercent.x  = 1.0f / (float)uWidth;
	vertex.xmf2FrameRatePercent.y  = 1.0f / (float)uHeight;

	m_cbInfo.m_xmf2Size			   = xmf2ObjSize;
	m_cbInfo.m_fFramePerTime       = dwFramePerTime;
	m_cbInfo.m_bMove			   = m_bMove;

	m_fDurability = (dwFramePerTime * dwFrameNum);// +0.01f;
}

void CTxAnimationObject::UpdateShaderVariable(ID3D11DeviceContext * pd3dDeviceContext)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dCSBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	CB_TX_ANIMATION_INFO *pcbParticle = (CB_TX_ANIMATION_INFO*)d3dMappedResource.pData;
	memcpy(pcbParticle, &m_cbInfo, sizeof(CB_TX_ANIMATION_INFO));
	pd3dDeviceContext->Unmap(m_pd3dCSBuffer, 0);
}

bool CTxAnimationObject::Enable(XMFLOAT3 * pos, int nColorNum)
{
	if (pos)
	{
		m_cbInfo.m_xmf3Pos = *pos;
		m_velocity.xmf3InitPos = *pos;
	}
	m_bEnable       = true;
	m_bTerminal     = false;
	m_bUseAnimation = true;

	m_cbInfo.m_fGameTime = 0.0;

	if (nColorNum != COLOR_NONE) m_cbInfo.m_nColorNum = nColorNum;

	return true;
}

bool CTxAnimationObject::Disable()
{
	cout << "exit time : " << m_cbInfo.m_fGameTime << endl;
	return (m_bEnable = false);
}

void CTxAnimationObject::NextEffectOn()
{
	m_pNextEffect->Enable(&m_cbInfo.m_xmf3Pos);
	m_bTerminal = true;
}

void CTxAnimationObject::Animate(float fTimeElapsed)
{
	if (m_bTerminal)
	{
		m_pNextEffect->Animate(fTimeElapsed);
		if (m_pNextEffect->IsTermainal())
			Disable();
	}
	else if(m_bUseAnimation)
	{
		m_cbInfo.m_fGameTime += fTimeElapsed;

		if (m_cbInfo.m_fGameTime > m_fDurability)
		{
			if (m_pNextEffect)  NextEffectOn();
			else Disable();
		}
		else if (m_bMove)
		{
			MoveUpdate(m_cbInfo.m_fGameTime, fTimeElapsed, m_cbInfo.m_xmf3Pos);
			m_cbInfo.m_xmf3LookVector = m_velocity.xmf3Velocity;
		}	
	}
}

void CTxAnimationObject::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	if (m_bTerminal)
	{
		m_pNextEffect->Render(pd3dDeviceContext, uRenderState, pCamera);
	}
	else
	{
		OnPrepare(pd3dDeviceContext);

		UpdateShaderVariable(pd3dDeviceContext);
		pd3dDeviceContext->GSSetConstantBuffers(0x04, 1, &m_pd3dCSBuffer);

		pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pd3dDrawVertexBuffer, &m_nVertexStrides, &m_nVertexOffsets);
		pd3dDeviceContext->Draw(1, 0);
	}
}

void CCircleMagic::Initialize(ID3D11Device * pd3dDevice)
{
	m_cbInfo.m_nColorNum = COLOR_GRAY;
	m_cbInfo.m_xmf3Pos   = { 0, 0, 0 };

	MoveVelocity move;
	move.xmf3Velocity    = { 0, 0, 0 };
	move.xmf3Accelate    = { 0, 0, 0 };
	move.fWeightSpeed    = 1.0f;
	SetMoveVelocity(move, &m_cbInfo.m_xmf3Pos);

	XMFLOAT2 xmf2Size      { 30, 30 };
	XMFLOAT2 xmf2ImageSize { 960, 768 };
	XMFLOAT2 xmf2FrameSize { 192, 192 };
	CTxAnimationObject::CreateBuffers(pd3dDevice, xmf2Size, xmf2ImageSize, xmf2FrameSize, 20, 0.05f);

	SetShaderResourceView(TXMgr.GetShaderResourceView("srv_sprite_circle.png"));
}

void CIceSpear::Initialize(ID3D11Device * pd3dDevice)
{
	m_cbInfo.m_nColorNum = COLOR_WHITE;
	m_cbInfo.m_xmf3Pos   = { 0, 0, 0 };

	MoveVelocity move;
	move.xmf3Velocity    = { 0, 0, 0 };
	move.xmf3Accelate    = { 0, 0, 0 };
	move.fWeightSpeed    = 1.0f;
	SetMoveVelocity(move, &m_cbInfo.m_xmf3Pos);

	XMFLOAT2 xmf2Size      { 20, 20 };
	XMFLOAT2 xmf2ImageSize { 960, 576 };
	XMFLOAT2 xmf2FrameSize { 192, 192 };
	CTxAnimationObject::CreateBuffers(pd3dDevice, xmf2Size, xmf2ImageSize, xmf2FrameSize, 13, 0.05f);

	SetShaderResourceView(TXMgr.GetShaderResourceView("srv_sprite_ice01.png"));
}

void CElementSpike::Initialize(ID3D11Device * pd3dDevice)
{
	m_cbInfo.m_nColorNum = COLOR_WHITE;
	m_cbInfo.m_xmf3Pos   = { 0, 0, 0 };

	MoveVelocity move;
	move.xmf3Velocity    = { 0, 0, 0 };
	move.xmf3Accelate    = { 0, 0, 0 };
	move.fWeightSpeed    = 1.0f;
	SetMoveVelocity(move, &m_cbInfo.m_xmf3Pos);

	XMFLOAT2 xmf2Size      { 20, 20 };
	XMFLOAT2 xmf2ImageSize { 640, 128 };
	XMFLOAT2 xmf2FrameSize { 128, 128 };
	CTxAnimationObject::CreateBuffers(pd3dDevice, xmf2Size, xmf2ImageSize, xmf2FrameSize, 5, 0.1f);
	m_cbInfo.m_bMove     = false;

	SetShaderResourceView(TXMgr.GetShaderResourceView("srv_sprite_spike.png"));
}

void CIceBolt::Initialize(ID3D11Device * pd3dDevice)
{
	m_cbInfo.m_nColorNum = COLOR_WHITE;
	m_cbInfo.m_xmf3Pos   = { 0, 0, 0 };

	MoveVelocity move;
	move.xmf3Velocity    = { 0, 0, 1 };
	move.xmf3Accelate    = { 0, 0, 1 };
	move.fWeightSpeed    = 10.0f;
	SetMoveVelocity(move, &m_cbInfo.m_xmf3Pos);

	XMFLOAT2 xmf2Size      { 10, 10 };
	XMFLOAT2 xmf2ImageSize { 128, 128 };
	XMFLOAT2 xmf2FrameSize { 128, 128 };
	CTxAnimationObject::CreateBuffers(pd3dDevice, xmf2Size, xmf2ImageSize, xmf2FrameSize, 1, 5.0f);
	m_cbInfo.m_bMove     = false;

	SetShaderResourceView(TXMgr.GetShaderResourceView("srv_sprite_ice_bolt.png"));

	m_pNextEffect = new CElementSpike();
	m_pNextEffect->Initialize(pd3dDevice);
}

void CElectricBolt::Initialize(ID3D11Device * pd3dDevice)
{
	m_cbInfo.m_nColorNum = COLOR_WHITE;
	m_cbInfo.m_xmf3Pos   = { 0, 0, 0 };

	MoveVelocity move;
	move.xmf3Velocity    = { 0, 0, 1 };
	move.xmf3Accelate    = { 0, 0, 1 };
	move.fWeightSpeed    = 10.0f;
	SetMoveVelocity(move, &m_cbInfo.m_xmf3Pos);

	XMFLOAT2 xmf2Size      { 8, 8 };
	XMFLOAT2 xmf2ImageSize { 128, 128 };
	XMFLOAT2 xmf2FrameSize { 128, 128 };
	CTxAnimationObject::CreateBuffers(pd3dDevice, xmf2Size, xmf2ImageSize, xmf2FrameSize, 1, 5.0f);
	m_cbInfo.m_bMove     = false;

	SetShaderResourceView(TXMgr.GetShaderResourceView("srv_sprite_electric_bolt.png"));

	m_pNextEffect = new CElementSpike();
	m_pNextEffect->Initialize(pd3dDevice);
}

////////////////////////////////////////////////////////////////////////////////////////
CParticle::CParticle() : CEffect()
{
	m_pd3dInitialVertexBuffer   = nullptr;
	m_pd3dStreamOutVertexBuffer = nullptr;

	m_pcNextParticle            = nullptr;

	ZeroMemory(&m_cbParticle, sizeof(CB_PARTICLE));

	m_bInitilize   = false;

	m_pd3dCSParticleBuffer = nullptr;
}

CParticle::~CParticle()
{
	if (m_pd3dStreamOutVertexBuffer) m_pd3dStreamOutVertexBuffer->Release();
	if (m_pd3dInitialVertexBuffer) m_pd3dInitialVertexBuffer->Release();

	if (m_pcNextParticle) delete m_pcNextParticle;
	if (m_pd3dCSParticleBuffer) m_pd3dCSParticleBuffer->Release();
}

void CParticle::Initialize(ID3D11Device *pd3dDevice)
{
	CB_PARTICLE cbParticle;
	ZeroMemory(&cbParticle, sizeof(CB_PARTICLE));
	MoveVelocity mov = MoveVelocity();

	CParticle::SetParticle(cbParticle, mov, 1.0f, 200.0f);

	PARTICLE_INFO cParticle;
	ZeroMemory(&cParticle, sizeof(PARTICLE_INFO));

	CParticle::CreateParticleBuffer(pd3dDevice, cParticle, 200.0f);
}

void CParticle::SetParticle(CB_PARTICLE & info, MoveVelocity & Velocity, float fDurability, UINT uMaxParticle)
{
	SetDurabilityTime(fDurability);
	Velocity.xmf3InitPos = m_cbParticle.m_vParticleEmitPos;

	m_bEnable              = false;
	m_bInitilize           = true;
	ZeroMemory(&m_cbParticle, sizeof(CB_PARTICLE));

	m_cbParticle           = info;
	m_cbParticle.m_bEnable = 1.0f;
	SetMoveVelocity(Velocity, &m_cbParticle.m_vParticleEmitPos);
}

void CParticle::CreateParticleBuffer(ID3D11Device * pd3dDevice, PARTICLE_INFO & pcInfo, UINT nMaxNum)
{
	m_nVertexStrides = sizeof(PARTICLE_INFO);

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = m_nVertexStrides;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;// | D3D11_BIND_STREAM_OUTPUT;

	HRESULT hr = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = &pcInfo;// pQuadPatchVertices;
	ASSERT_S(hr = pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dInitialVertexBuffer));

	d3dBufferDesc.ByteWidth = m_nVertexStrides * nMaxNum;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;

	// SO으로 받을 버퍼는 서브리소스 데이터 값을 nullptr로 해야한다!!
	ASSERT_S(hr = pd3dDevice->CreateBuffer(&d3dBufferDesc, nullptr, &m_pd3dStreamOutVertexBuffer));
	ASSERT_S(hr = pd3dDevice->CreateBuffer(&d3dBufferDesc, nullptr, &m_pd3dDrawVertexBuffer));

	m_pd3dCSParticleBuffer = ViewMgr.GetBuffer("cs_particle");
	m_pd3dCSParticleBuffer->AddRef();
	//D3D11_QUERY_DESC qd;
	//qd.Query = D3D11_QUERY_SO_STATISTICS;
	//qd.MiscFlags = D3D11_QUERY_MISC_PREDICATEHINT;

	//ASSERT_S(pd3dDevice->CreateQuery(&qd, &m_pd3dQuery));
}

void CParticle::StreamOut(ID3D11DeviceContext *pd3dDeviceContext)
{
	pd3dDeviceContext->SOSetTargets(1, &m_pd3dStreamOutVertexBuffer, &m_nVertexOffsets);
	// 원인은 무엇인가??
	static const UINT strides[] = { (m_nVertexStrides * 3) };

	if (m_bTerminal)
		m_pcNextParticle->StreamOut(pd3dDeviceContext);
	else
	{
		UpdateShaderVariable(pd3dDeviceContext);
		pd3dDeviceContext->GSSetConstantBuffers(0x04, 1, &m_pd3dCSParticleBuffer);

		if (m_bInitilize)
		{
			pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pd3dInitialVertexBuffer, strides, &m_nVertexOffsets);
			pd3dDeviceContext->Draw(1, 0);
			m_bInitilize = false;
		}
		else
		{
			pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pd3dDrawVertexBuffer, strides, &m_nVertexOffsets);
			pd3dDeviceContext->DrawAuto();
		}
	}
}

void CParticle::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	if (m_bTerminal)
	{
		m_pcNextParticle->Render(pd3dDeviceContext, uRenderState, pCamera);
	}
	else
	{
		swap(m_pd3dDrawVertexBuffer, m_pd3dStreamOutVertexBuffer);

		OnPrepare(pd3dDeviceContext);
		UpdateShaderVariable(pd3dDeviceContext);
		pd3dDeviceContext->VSSetConstantBuffers(0x04, 1, &m_pd3dCSParticleBuffer);

		pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pd3dDrawVertexBuffer, &m_nVertexStrides, &m_nVertexOffsets);
		pd3dDeviceContext->DrawAuto();
	}
}

void CParticle::UpdateShaderVariable(ID3D11DeviceContext * pd3dDeviceContext)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dCSParticleBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	CB_PARTICLE *pcbParticle = (CB_PARTICLE *)d3dMappedResource.pData;
	memcpy(pcbParticle, GetCBParticle(), sizeof(CB_PARTICLE));
	pd3dDeviceContext->Unmap(m_pd3dCSParticleBuffer, 0);
}

void CParticle::Update(float fTimeElapsed)
{
	if (m_bTerminal)
	{
		m_pcNextParticle->Update(fTimeElapsed);
		if (m_pcNextParticle->IsTermainal())
			Disable();
	}
	else
	{
		m_cbParticle.m_fGameTime += fTimeElapsed;
		m_cbParticle.m_fTimeStep  = fTimeElapsed;
		float fGameTime = m_cbParticle.m_fGameTime;

		LifeUpdate(fGameTime, fTimeElapsed);
		if (m_bMove && m_cbParticle.m_bEnable) 
			MoveUpdate(fGameTime, fTimeElapsed, m_cbParticle.m_vParticleEmitPos);
	}
}

void CParticle::LifeUpdate(const float & fGameTime, const float & fTimeElapsed)
{
	if (fGameTime > m_fDurability + m_cbParticle.m_fLifeTime)
	{
		m_cbParticle.m_fGameTime = 0.0f;
		if (!m_bTerminal) Disable();
	}
	else if (fGameTime > m_fDurability)
	{
		m_cbParticle.m_bEnable = 0;
		if (m_pcNextParticle)  NextParticleOn();
	}
}

bool CParticle::Enable(XMFLOAT3 * pos, int nColorNum)
{
	if (pos)
	{
		SetEmitPosition(*pos);
		m_velocity.xmf3InitPos = *pos;
	}

	m_bEnable                = true;
	m_bTerminal              = false;
	m_bInitilize             = true;
	m_cbParticle.m_bEnable   = 1;
	m_cbParticle.m_fGameTime = 0;
	
	if (nColorNum != COLOR_NONE) m_cbParticle.m_nColorNum = nColorNum;
	return true;
}

bool CParticle::Disable()
{
	return (m_bEnable = false);
}

void CParticle::NextParticleOn()
{
	m_pcNextParticle->Enable(&m_cbParticle.m_vParticleEmitPos);
	m_bTerminal = true;
}

void CSmokeBoomParticle::Initialize(ID3D11Device * pd3dDevice)
{
	static CB_PARTICLE cbParticle;
	ZeroMemory(&cbParticle, sizeof(CB_PARTICLE));

	cbParticle.m_fLifeTime         = 1.0f;
	cbParticle.m_vAccel            = XMFLOAT3(20, 20, 20);
	cbParticle.m_vParticleEmitPos  = XMFLOAT3(0, 0, 0);
	cbParticle.m_vParticleVelocity = XMFLOAT3(20, 20, 20);
	cbParticle.m_fNewTime          = 0.008f;
	cbParticle.m_fMaxSize          = 8.0f;
	cbParticle.m_nColorNum		   = COLOR_GRAY;

	MoveVelocity mov = MoveVelocity();

	CParticle::SetParticle(cbParticle, mov, 0.5f, m_nMaxParticlenum);

	PARTICLE_INFO cParticle;
	ZeroMemory(&cParticle, sizeof(PARTICLE_INFO));

	CParticle::CreateParticleBuffer(pd3dDevice, cParticle, m_nMaxParticlenum);
	SetShaderResourceView(TXMgr.GetShaderResourceView("srv_particle_smoke_array"));
}

void CFireBallParticle::Initialize(ID3D11Device * pd3dDevice)
{
	static CB_PARTICLE cbParticle;
	ZeroMemory(&cbParticle, sizeof(CB_PARTICLE));

	cbParticle.m_fLifeTime         = 1.0f;
	cbParticle.m_vAccel            = XMFLOAT3(-20, 20, -20);
	cbParticle.m_vParticleEmitPos  = XMFLOAT3(0, 0, 0);
	cbParticle.m_vParticleVelocity = XMFLOAT3(20, -20, 20);
	cbParticle.m_fNewTime          = 0.01f;
	cbParticle.m_fMaxSize          = 16.0f;
	cbParticle.m_nColorNum	       = COLOR_WHITE;

	MoveVelocity mov = MoveVelocity();
	mov.xmf3Velocity = XMFLOAT3(0, 0, 10);
	mov.xmf3Accelate = XMFLOAT3(0, 0, -10);
	mov.fWeightSpeed = 100.0f;

	CParticle::SetParticle(cbParticle, mov, 2.0f, m_nMaxParticlenum);

	PARTICLE_INFO cParticle;
	ZeroMemory(&cParticle, sizeof(PARTICLE_INFO));

	CParticle::CreateParticleBuffer(pd3dDevice, cParticle, m_nMaxParticlenum);
	SetShaderResourceView(TXMgr.GetShaderResourceView("srv_particle_fire_array"));

	m_pcNextParticle = new CSmokeBoomParticle();
	m_pcNextParticle->Initialize(pd3dDevice);
	CB_PARTICLE * pParticle        = m_pcNextParticle->GetCBParticle();
	pParticle->m_fMaxSize          = 8.0f;
	pParticle->m_fNewTime          = 0.0005f;
	pParticle->m_vParticleVelocity = XMFLOAT3(20, 20, 20);
	pParticle->m_vAccel            = XMFLOAT3(40, 40, 40);
	pParticle->m_nColorNum         = COLOR_GRAY;
}
//
//void CRainParticle::StreamOut(ID3D11DeviceContext * pd3dDeviceContext)
//{
//	pd3dDeviceContext->SOSetTargets(1, &m_pd3dStreamOutVertexBuffer, &m_nVertexOffsets);
//	// 원인은 무엇인가??
//	static const UINT strides[] = { (m_nVertexStrides) };
//	//pd3dDeviceContext->Begin(m_pd3dQuery);
//
//	UpdateShaderVariable(pd3dDeviceContext);
//	if (m_bInitilize)
//	{
//		pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pd3dInitialVertexBuffer, strides, &m_nVertexOffsets);
//		pd3dDeviceContext->Draw(1, 0);
//		m_bInitilize = false;
//	}
//	else
//	{
//		pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pd3dDrawVertexBuffer, strides, &m_nVertexOffsets);
//		pd3dDeviceContext->DrawAuto();
//	}
//}

void CRainParticle::Initialize(ID3D11Device * pd3dDevice)
{
	CB_PARTICLE cbParticle;
	ZeroMemory(&cbParticle, sizeof(CB_PARTICLE));

	cbParticle.m_fLifeTime         = 1.0f;
	cbParticle.m_vAccel            = XMFLOAT3(0, -40, 0);
	cbParticle.m_vParticleEmitPos  = XMFLOAT3(512, 380, 256);
	cbParticle.m_vParticleVelocity = XMFLOAT3(0, -300, 0);
	cbParticle.m_fNewTime          = 0.01f;
	cbParticle.m_fMaxSize          = 10.0f;
	cbParticle.m_nColorNum		   = COLOR_WHITE;

	MoveVelocity mov = MoveVelocity();

	CParticle::SetParticle(cbParticle, mov, 100.0f, m_nMaxParticlenum);

	PARTICLE_INFO cParticle;
	ZeroMemory(&cParticle, sizeof(PARTICLE_INFO));

	CParticle::CreateParticleBuffer(pd3dDevice, cParticle, m_nMaxParticlenum);
}

//////////////////////////////////////
CAbsorbMarble::CAbsorbMarble() : CBillboardObject()
{
	Initialize();
}

CAbsorbMarble::CAbsorbMarble(XMFLOAT3 pos, UINT fID, XMFLOAT2 xmf2Size) : CBillboardObject(pos, fID, xmf2Size)
{
	Initialize();
}

CAbsorbMarble::~CAbsorbMarble()
{
}

void CAbsorbMarble::Initialize()
{
	SetCollide(true);

	m_bAbsorb          = false;
	m_fAbsorbTime      = 0.0f;
	m_fSpeed           = 0.0f;
	m_pTargetObject    = nullptr;

	m_xvRandomVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

void CAbsorbMarble::SetTarget(CGameObject * pGameObject)
{
	if (!m_bAbsorb)
	{
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
		XMVECTOR xmvTarget   = XMLoadFloat3(&m_pTargetObject->GetPosition());
		XMVECTOR xvPos       = XMLoadFloat3(&CBillboardObject::GetPosition());
		XMVECTOR xmvToTarget = xmvTarget - xvPos;
		xmvToTarget          = XMVector3Normalize(xmvToTarget) * 2.0f;

		XMVECTOR xvSpeed     = XMVectorReplicate(m_fSpeed);
		XMVECTOR xvRandom    = XMLoadFloat3(&m_xvRandomVelocity);

		xvPos = (0.5f * xmvToTarget * m_fAbsorbTime * m_fAbsorbTime) + (xvRandom * m_fAbsorbTime) + xvPos;
		XMFLOAT3 xmfPos;
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

void CAbsorbMarble::GetGameMessage(CGameObject * byObj, eMessage eMSG, void * extra)
{
	switch (eMSG)
	{
	case eMessage::MSG_CULL_IN:
		m_bActive = true;
		return;
	case eMessage::MSG_CULL_OUT:
		m_bActive = false;
		return;
	case eMessage::MSG_COLLIDE:
		m_pTargetObject->GetGameMessage(this, eMessage::MSG_GET_SOUL, &GetInstanceData());
		SetPosition(XMFLOAT3(rand() % 2048, 100, rand() % 2048));
		Initialize();
		SetCollide(false);
		//EVENTMgr.InsertDelayMessage(1.0f, eMessage::MSG_OBJECT_RENEW, CGameEventMgr::MSG_TYPE_OBJECT, this);
		return;
	case eMessage::MSG_COLLIDED:
		SetTarget(byObj);
		return;

	//case eMessage::MSG_QUAD_DELETE:

	//	return;
	case eMessage::MSG_OBJECT_RENEW:
		CBillboardObject::UpdateInstanceData();
		SetCollide(true);
		QUADMgr.EntityStaticObject(this);
		return;
	case eMessage::MSG_NORMAL:
		return;
	}
}

bool CAbsorbMarble::IsVisible(CCamera *pCamera)
{
	bool bIsVisible = m_bActive;

	if (pCamera)
		bIsVisible = CBillboardObject::IsVisible(pCamera);
	else if (!bIsVisible || m_bAbsorb)
		bIsVisible = m_bActive = true;

	return(bIsVisible);
}
