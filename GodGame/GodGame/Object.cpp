#include "stdafx.h"
#include "MyInline.h"
#include "Object.h"
#include "Shader.h"

//////////////////////////////////////////////////////////////////////////

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

	m_nReferences = 0;
	m_pMaterial   = nullptr;
	m_pTexture    = nullptr;
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
	if (m_nReferences > 0) m_nReferences--;
	if (m_nReferences <= 0) delete this;
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
		m_bcMeshBoundingCube.Update(m_xmf44World, nullptr);

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
			if (m_bActive) m_ppMeshes[i]->Render(pd3dDeviceContext, uRenderState);
			m_bActive = false;
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

XMFLOAT3 CGameObject::GetLookAt()
{
	//게임 객체를 로컬 z-축 벡터를 반환한다.
	XMFLOAT3 xv3LookAt(m_xmf44World._31, m_xmf44World._32, m_xmf44World._33);
	Chae::XMFloat3Normalize(&xv3LookAt);
	return(xv3LookAt);
}
XMFLOAT3 CGameObject::GetUp()
{
	//게임 객체를 로컬 y-축 벡터를 반환한다.
	XMFLOAT3 xv3Up(m_xmf44World._21, m_xmf44World._22, m_xmf44World._23);
	Chae::XMFloat3Normalize(&xv3Up);
	return(xv3Up);
}
XMFLOAT3 CGameObject::GetRight()
{
	//게임 객체를 로컬 x-축 벡터를 반환한다.
	XMFLOAT3 xv3Right(m_xmf44World._11, m_xmf44World._12, m_xmf44World._13);
	Chae::XMFloat3Normalize(&xv3Right);
	return(xv3Right);
}

void CGameObject::GetGameMessage(CGameObject * byObj, eMessage eMSG)
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

void CGameObject::SendGameMessage(CGameObject * toObj, eMessage eMSG)
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

void CGameObject::MessageObjToObj(CGameObject * byObj, CGameObject * toObj, eMessage eMSG)
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
	m_pHeightMapImage = nullptr;
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

//void CBillboardObject::UpdateBoundingBox()
//{
//	XMFLOAT3 pos = GetPosition();
//
//	m_bcMeshBoundingCube.m_xv3Maximum = XMFLOAT3(pos.x + (m_xv2Size.x * 0.5f), pos.y + (m_xv2Size.y * 0.5f), pos.z + 1);
//	m_bcMeshBoundingCube.m_xv3Minimum = XMFLOAT3(pos.x - (m_xv2Size.x * 0.5f), pos.y - (m_xv2Size.y * 0.5f), pos.z - 1);
//}

void CBillboardObject::SetPosition(XMFLOAT3 & xv3Position)
{
	XMFLOAT3 pos = xv3Position;
	CGameObject::SetPosition(pos);
	m_xv4InstanceData.x = pos.x;
	m_xv4InstanceData.y = pos.y;
	m_xv4InstanceData.z = pos.z;
	//m_bcMeshBoundingCube.m_xv3Maximum = XMFLOAT3(pos.x + (m_xv2Size.x * 0.5f), pos.y + (m_xv2Size.y * 0.5f), pos.z + 1);
	//m_bcMeshBoundingCube.m_xv3Minimum = XMFLOAT3(pos.x - (m_xv2Size.x * 0.5f), pos.y - (m_xv2Size.y * 0.5f), pos.z - 1);
}

bool CBillboardObject::IsVisible(CCamera *pCamera)
{
	bool bIsVisible = m_bActive;

	if (pCamera)
	{
		m_bcMeshBoundingCube.Update(m_xmf44World, &m_ppMeshes[0]->GetBoundingCube());
	//	AABB BoundingBox = m_bcMeshBoundingCube;
		//XMVECTOR xmvMin = XMLoadFloat3(&BoundingBox.m_xv3Minimum);
		//XMVECTOR xmvMax = XMLoadFloat3(&BoundingBox.m_xv3Maximum);
		//XMVECTOR xmvPos = XMLoadFloat4(&m_xv4InstanceData);

		//xmvMin += xmvPos;
		//xmvMax += xmvPos;

		//XMStoreFloat3(&BoundingBox.m_xv3Maximum, xmvMax);
		//XMStoreFloat3(&BoundingBox.m_xv3Minimum, xmvMin);
		//m_BoundingBox.Update(m_xmf44World, &m_BoundingBox);
		bIsVisible = pCamera->IsInFrustum(&m_bcMeshBoundingCube);
	}

	XMFLOAT3 xmfPos = GetPosition();
	m_xv4InstanceData.x = xmfPos.x;
	m_xv4InstanceData.y = xmfPos.y;
	m_xv4InstanceData.z = xmfPos.z;

	return(bIsVisible);
}

CParticle::CParticle()
{
	m_pd3dInitialVertexBuffer = nullptr;
	m_pd3dStreamOutVertexBuffer = nullptr;
	m_pd3dDrawVertexBuffer = nullptr;

	m_nVertices = 0;
	m_nStartVertex = 0;
	m_nVertexStrides = 0;
	m_nVertexOffsets = 0;

	m_fDurability = 0;

	ZeroMemory(&m_cbParticle, sizeof(CB_PARTICLE));
	ZeroMemory(&m_cParticle, sizeof(PATICLE_INFO));

	m_bInitilize = false;
	m_bEnable = false;

	m_pd3dQuery = nullptr;
}

CParticle::~CParticle()
{
	if (m_pd3dDrawVertexBuffer)	m_pd3dDrawVertexBuffer->Release();
	if (m_pd3dStreamOutVertexBuffer) m_pd3dStreamOutVertexBuffer->Release();
	if (m_pd3dInitialVertexBuffer) m_pd3dInitialVertexBuffer->Release();
}

void CParticle::Initialize(ID3D11Device *pd3dDevice, CB_PARTICLE & info, float fDurability, int iMaxParticle)
{
	m_fDurability = fDurability;

	m_bEnable = true;
	m_bInitilize = true;

	m_nVertices = 1;
	ZeroMemory(&m_cParticle, sizeof(CB_PARTICLE));
	m_cParticle.m_xmf3Pos = info.m_vParticleEmitPos;
	m_cParticle.m_uType = PARTICLE_TYPE_EMITTER;

	m_cbParticle.m_vParticleVelocity = info.m_vParticleVelocity;
	m_cbParticle.m_vAccel = info.m_vAccel;
	m_cbParticle.m_vParticleEmitPos = info.m_vParticleEmitPos;
	m_cbParticle.m_fLifeTime = info.m_fLifeTime;
	m_cbParticle.m_fNewTime = info.m_fNewTime;
	m_cbParticle.m_bEnable = 1;
	m_cbParticle.m_NewSize = XMFLOAT2(4, 4);

	m_nVertexStrides = sizeof(PATICLE_INFO);
	//cout << m_nVertexStrides << endl;

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(PATICLE_INFO) * 2;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;// | D3D11_BIND_STREAM_OUTPUT;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = &m_cParticle;// pQuadPatchVertices;
	HRESULT hr = pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dInitialVertexBuffer);
	if (FAILED(hr)) printf("버퍼생성 실패");

	d3dBufferDesc.ByteWidth = sizeof(PATICLE_INFO) * iMaxParticle;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;

	// SO으로 받을 버퍼는 서브리소스 데이터 값을 nullptr로 해야한다!!
	hr = pd3dDevice->CreateBuffer(&d3dBufferDesc, nullptr, &m_pd3dStreamOutVertexBuffer);
	if (FAILED(hr)) printf("버퍼생성 실패");
	hr = pd3dDevice->CreateBuffer(&d3dBufferDesc, nullptr, &m_pd3dDrawVertexBuffer);
	if (FAILED(hr)) printf("버퍼생성 실패");

	D3D11_QUERY_DESC qd;
	qd.Query = D3D11_QUERY_SO_STATISTICS;
	qd.MiscFlags = D3D11_QUERY_MISC_PREDICATEHINT;

	//HRESULT hr = pd3dDevice->CreateQuery(&qd, &m_pd3dQuery);
	//if (FAILED(hr)) printf("실패했습니다.");
}

void CParticle::StreamOut(ID3D11DeviceContext *pd3dDeviceContext)
{
	pd3dDeviceContext->SOSetTargets(1, &m_pd3dStreamOutVertexBuffer, &m_nVertexOffsets);
	UINT strides[] = { m_nVertexStrides * 2 };
	//pd3dDeviceContext->Begin(m_pd3dQuery);
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
	//pd3dDeviceContext->End(m_pd3dQuery);
	//D3D11_QUERY_DATA_SO_STATISTICS d3dSOStatics;
	//pd3dDeviceContext->GetData(m_pd3dQuery, &d3dSOStatics, sizeof(D3D11_QUERY_DATA_SO_STATISTICS), 0);
	//printf("Num: %u //", d3dSOStatics.NumPrimitivesWritten);
	//printf("Storage %u \n", d3dSOStatics.PrimitivesStorageNeeded);
}

void CParticle::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	ID3D11Buffer * pd3dBuffer = m_pd3dDrawVertexBuffer;
	m_pd3dDrawVertexBuffer = m_pd3dStreamOutVertexBuffer;
	m_pd3dStreamOutVertexBuffer = pd3dBuffer;

	UINT strides[] = { m_nVertexStrides };
	ID3D11Buffer * pd3dBuffers[1] = { nullptr };
	pd3dDeviceContext->SOSetTargets(1, pd3dBuffers, 0);
	pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pd3dDrawVertexBuffer, strides, &m_nVertexOffsets);
	pd3dDeviceContext->DrawAuto();

	//pd3dDeviceContext->IASetVertexBuffers(0, 1, &m_pd3dInitialVertexBuffer, Strides, Offsets);
	//pd3dDeviceContext->Draw(1, 0);
}

void CParticle::Update(float fTimeElapsed)
{
	m_cbParticle.m_fGameTime += fTimeElapsed;
	m_cbParticle.m_fTimeStep = fTimeElapsed;
	m_cbParticle.m_NewSize = XMFLOAT2(rand() % 4 * 2, rand() % 4 * 2);

	if (m_cbParticle.m_fGameTime > m_fDurability + m_cbParticle.m_fLifeTime)
	{
		m_cbParticle.m_fGameTime = 0.0f;
		Disable();
	}
	else if (m_cbParticle.m_fGameTime > m_fDurability)
	{
		m_cbParticle.m_bEnable = 0;
	}
}

void CParticle::Enable()
{
	m_bEnable = true;
	m_bInitilize = true;
	m_cbParticle.m_bEnable = 1;
}

void CParticle::Disable()
{
	m_bEnable = false;
}

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
		m_pTargetObject = pGameObject;
		m_bAbsorb = true;

		XMVECTOR xmvTarget = XMLoadFloat3(&m_pTargetObject->GetPosition());
		XMVECTOR xvPos = XMLoadFloat3(&CBillboardObject::GetPosition());
		XMVECTOR xmvFromTarget = xvPos - xmvTarget;
		xmvFromTarget = XMVector3Normalize(xmvFromTarget);
		XMStoreFloat3(&m_xvRandomVelocity, xmvFromTarget);

		m_xvRandomVelocity.y = abs(m_xvRandomVelocity.y);
		//m_xvRandomVelocity.x = Chae::RandomFloat(0, 1);
		//m_xvRandomVelocity.y = Chae::RandomFloat(0, 1);
		//m_xvRandomVelocity.z = Chae::RandomFloat(0, 1);

		m_fSpeed = rand() % 10 + 10.0f;

	}
}

void CAbsorbMarble::Animate(float fTimeElapsed)
{
	if (m_bAbsorb)
	{
		m_fAbsorbTime += fTimeElapsed;
		XMVECTOR xmvTarget = XMLoadFloat3(&m_pTargetObject->GetPosition());
		XMVECTOR xvPos = XMLoadFloat3(&CBillboardObject::GetPosition());
		XMVECTOR xmvToTarget = xmvTarget - xvPos;
		xmvToTarget = XMVector3Normalize(xmvToTarget) * 2.0f;

		XMVECTOR xvSpeed = XMVectorReplicate(m_fSpeed);
		XMVECTOR xvRandom = XMLoadFloat3(&m_xvRandomVelocity);

		xvPos = (0.5f * xmvToTarget * m_fAbsorbTime * m_fAbsorbTime) + (xvRandom * m_fAbsorbTime) + xvPos;
		XMFLOAT3 xmfPos;
		XMStoreFloat3(&xmfPos, xvPos);
		CBillboardObject::SetPosition(xmfPos);

		XMVECTOR lengthSq = XMVector3LengthSq(xmvTarget - xvPos);
		float flengthSq;
		XMStoreFloat(&flengthSq, lengthSq);

		if (flengthSq < 100.0f && m_fAbsorbTime > 0.3f)
		{
			GetGameMessage(this, eMessage::MSG_COLLIDE);
		}
	}
}

void CAbsorbMarble::GetGameMessage(CGameObject * byObj, eMessage eMSG)
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
		SetPosition(XMFLOAT3(rand() % 2048, 100, rand() % 2048));
		Initialize();
		QUADMgr.EntityStaticObject(this);
		return;
	case eMessage::MSG_COLLIDED:
		SetTarget(byObj);
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