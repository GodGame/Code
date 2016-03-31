#include "stdafx.h"
#include "MyInline.h"
//#include "Object.h"
//#include "Character.h"
#include "Player.h"

CPlayer::CPlayer(int nMeshes) : CCharacter(nMeshes)
{
	m_pCamera               = nullptr;

	m_xv3Position           = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//m_xv3Position.z +     = 1;
	m_xv3Right              = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xv3Up                 = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xv3Look               = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xv3Gravity            = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ        = 0.0f;
	m_fMaxVelocityY         = 0.0f;
	m_fFriction             = 0.0f;

	m_fPitch                = 0.0f;
	m_fRoll                 = 0.0f;
	m_fYaw                  = 0.0f;

	m_pPlayerUpdatedContext = nullptr;
	m_pCameraUpdatedContext = nullptr;

	m_pScene = nullptr;
}

CPlayer::~CPlayer()
{
	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
}

void CPlayer::UpdateShaderVariables(ID3D11DeviceContext *pd3dDeviceContext)
{
	//플레이어의 현재 카메라의 UpdateShaderVariables() 멤버 함수를 호출한다.
	if (m_pCamera) m_pCamera->UpdateShaderVariables(pd3dDeviceContext, m_pCamera->GetViewProjectionMatrix(), m_pCamera->GetPosition());
	//printf("Player : %0.2f %0.2f %0.2f \n", m_xmf44World._41, m_xmf44World._42, m_xmf44World._43);
	//cout << "player" << endl;
	//cout << "bb max : " << m_bcMeshBoundingCube.m_xv3Maximum.x << ", " << m_bcMeshBoundingCube.m_xv3Maximum.y << ", " << m_bcMeshBoundingCube.m_xv3Maximum.z << endl;
	//cout << "bb min : " << m_bcMeshBoundingCube.m_xv3Minimum.x << ", " << m_bcMeshBoundingCube.m_xv3Minimum.y << ", " << m_bcMeshBoundingCube.m_xv3Minimum.z << endl;
}

void CPlayer::SetPosition(XMFLOAT3& xv3Position)
{
	XMFLOAT3 xv3Result;
	Chae::XMFloat3Sub(&xv3Result, &xv3Position, &m_xv3Position);
	Move(xv3Result, false);
}

void CPlayer::InitPosition(XMFLOAT3 xv3Position)
{
	m_xv3Position.x = xv3Position.x;
	m_xv3Position.y = xv3Position.y;
	m_xv3Position.z = xv3Position.z;
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		WORD wdNextState = 0;
		XMVECTOR xv3Shift = XMVectorSet(0, 0, 0, 0);
		//화살표 키 ‘↑’를 누르면 로컬 z-축 방향으로 이동(전진)한다. ‘↓’를 누르면 반대 방향으로 이동한다.
		if (dwDirection & DIR_FORWARD)
		{
			xv3Shift += XMLoadFloat3(&m_xv3Look) * fDistance * 1.2f;
			wdNextState = eANI_RUN_FORWARD;
		}
		if (dwDirection & DIR_BACKWARD)
		{
			xv3Shift -= XMLoadFloat3(&m_xv3Look) * fDistance;
			wdNextState = eANI_WALK_BACK;
		}
		//화살표 키 ‘→’를 누르면 로컬 x-축 방향으로 이동한다. ‘←’를 누르면 반대 방향으로 이동한다.
		if (dwDirection & DIR_RIGHT)
		{
			xv3Shift += XMLoadFloat3(&m_xv3Right) * fDistance;
			wdNextState = eANI_WALK_RIGHT;
		}
		if (dwDirection & DIR_LEFT)
		{
			xv3Shift -= XMLoadFloat3(&m_xv3Right) * fDistance;
			wdNextState = eANI_WALK_LEFT;
		}
		//‘Page Up’을 누르면 로컬 y-축 방향으로 이동한다. ‘Page Down’을 누르면 반대 방향으로 이동한다.
		if (dwDirection & DIR_UP) xv3Shift += XMLoadFloat3(&m_xv3Up) * fDistance;
		if (dwDirection & DIR_DOWN) xv3Shift -= XMLoadFloat3(&m_xv3Up) * fDistance;

		XMFLOAT3 xmf3Shift;
		XMStoreFloat3(&xmf3Shift, xv3Shift);

		if (wdNextState != m_wdAnimateState)
		{
			m_wdAnimateState = wdNextState;
			CAnimatedMesh* pAnimatedMesh = static_cast<CAnimatedMesh*>(m_ppMeshes[m_wdAnimateState]);
			
			//XMVECTOR max = XMLoadFloat3(&pAnimatedMesh->GetAABBMesh().m_xv3Maximum);
			//XMVECTOR min = XMLoadFloat3(&pAnimatedMesh->GetAABBMesh().m_xv3Minimum);
			//max = (max + min) * 0.5f;

			//XMFLOAT3 Pos = GetPosition();
			//XMStoreFloat3(&Pos, max + XMLoadFloat3(&Pos));
			//SetPosition(Pos);

			pAnimatedMesh->ResetIndex();
		}
		//플레이어를 현재 위치 벡터에서 xv3Shift 벡터 만큼 이동한다.
		Move(xmf3Shift, bUpdateVelocity);
	}
	else
	{
		if (m_wdAnimateState != eANI_IDLE)
		{
			m_wdAnimateState = eANI_IDLE;
			CAnimatedMesh* pAnimatedMesh = static_cast<CAnimatedMesh*>(m_ppMeshes[m_wdAnimateState]);
			pAnimatedMesh->ResetIndex();
		}
	}
}
void CPlayer::Move(XMFLOAT3& xv3Shift, bool bUpdateVelocity)
{
	//bUpdateVelocity가 참이면 플레이어를 이동하지 않고 속도 벡터를 변경한다.
	if (bUpdateVelocity)
	{
		m_xv3Velocity.x += xv3Shift.x;
		m_xv3Velocity.y += xv3Shift.y;
		m_xv3Velocity.z += xv3Shift.z;
	}
	else
	{
		//플레이어를 현재 위치 벡터에서 xv3Shift 벡터 만큼 이동한다.
		XMFLOAT3 xv3Position;
		xv3Position.x = m_xv3Position.x + xv3Shift.x;
		xv3Position.y = m_xv3Position.y + xv3Shift.y;
		xv3Position.z = m_xv3Position.z + xv3Shift.z;

		m_xv3Position = xv3Position;
		//플레이어의 위치가 변경되었으므로 카메라의 위치도 xv3Shift 벡터 만큼 이동한다.
		m_pCamera->Move(xv3Shift);
	}
}

//플레이어를 로컬 x-축, y-축, z-축을 중심으로 회전한다.
void CPlayer::Rotate(float x, float y, float z)
{
	XMVECTOR xmvRight = XMLoadFloat3(&m_xv3Right);
	XMVECTOR xmvUp = XMLoadFloat3(&m_xv3Up);
	XMVECTOR xmvLook = XMLoadFloat3(&m_xv3Look);

	XMMATRIX mtxRotate;
	DWORD nCurrentCameraMode = m_pCamera->GetMode();

	//1인칭 카메라 또는 3인칭 카메라의 경우 플레이어의 회전은 약간의 제약이 따른다.
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA))
	{
		/*로컬 x-축을 중심으로 회전하는 것은 고개를 앞뒤로 숙이는 동작에 해당한다. 그러므로 x-축을 중심으로 회전하는 각도는 -89.0~+89.0도 사이로 제한한다. x는 현재의 m_fPitch에서 실제 회전하는 각도이므로 x만큼 회전한 다음 Pitch가 +89도 보다 크거나 -89도 보다 작으면 m_fPitch가 +89도 또는 -89도가 되도록 회전각도(x)를 수정한다.*/
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		//로컬 y-축을 중심으로 회전하는 것은 몸통을 돌리는 것이므로 회전 각도의 제한이 없다.
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		/*로컬 z-축을 중심으로 회전하는 것은 몸통을 좌우로 기울이는 것이므로 회전 각도는 -20.0~+20.0도 사이로 제한된다. z는 현재의 m_fRoll에서 실제 회전하는 각도이므로 z만큼 회전한 다음 m_fRoll이 +20도 보다 크거나 -20도보다 작으면 m_fRoll이 +20도 또는 -20도가 되도록 회전각도(z)를 수정한다.*/
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		//카메라를 x, y, z 만큼 회전한다. 플레이어를 회전하면 카메라가 회전하게 된다.
		m_pCamera->Rotate(x, y, z);

		/*플레이어를 회전한다. 1인칭 카메라 또는 3인칭 카메라에서 플레이어의 회전은 로컬 y-축에서만 일어난다. 플레이어의 로컬 y-축(Up 벡터)을 기준으로 로컬 z-축(Look 벡터)와 로컬 x-축(Right 벡터)을 회전시킨다. 기본적으로 Up 벡터를 기준으로 회전하는 것은 플레이어가 똑바로 서있는 것을 가정한다는 의미이다.*/
		if (y != 0.0f)
		{
			mtxRotate = XMMatrixRotationAxis(xmvUp, (float)XMConvertToRadians(y));
			xmvLook = XMVector3TransformNormal(xmvLook, mtxRotate);
			xmvRight = XMVector3TransformNormal(xmvRight, mtxRotate);
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		/*스페이스-쉽 카메라에서 플레이어의 회전은 회전 각도의 제한이 없다. 그리고 모든 축을 중심으로 회전을 할 수 있다.*/
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			mtxRotate = XMMatrixRotationAxis(xmvRight, (float)XMConvertToRadians(x));
			xmvLook = XMVector3TransformNormal(xmvLook, mtxRotate);
			xmvUp = XMVector3TransformNormal(xmvUp, mtxRotate);
		}
		if (y != 0.0f)
		{
			mtxRotate = XMMatrixRotationAxis(xmvUp, (float)XMConvertToRadians(y));
			xmvLook = XMVector3TransformNormal(xmvLook, mtxRotate);
			xmvRight = XMVector3TransformNormal(xmvRight, mtxRotate);
		}
		if (z != 0.0f)
		{
			mtxRotate = XMMatrixRotationAxis(xmvLook, (float)XMConvertToRadians(z));
			xmvUp = XMVector3TransformNormal(xmvUp, mtxRotate);
			xmvRight = XMVector3TransformNormal(xmvRight, mtxRotate);
		}
	}

	/*회전으로 인해 플레이어의 로컬 x-축, y-축, z-축이 서로 직교하지 않을 수 있으므로 z-축(LookAt 벡터)을 기준으로 하여 서로 직교하고 단위벡터가 되도록 한다.*/
	xmvLook  = XMVector3Normalize(xmvLook);
	xmvRight = XMVector3Cross(xmvUp, xmvLook);
	xmvRight = XMVector3Normalize(xmvRight);
	xmvUp    = XMVector3Cross(xmvLook, xmvRight);
	xmvUp    = XMVector3Normalize(xmvUp);

	XMStoreFloat3(&m_xv3Right, xmvRight);
	XMStoreFloat3(&m_xv3Up, xmvUp);
	XMStoreFloat3(&m_xv3Look, xmvLook);
}

void CPlayer::Rotate(XMFLOAT3 & xmf3RotAxis, float fAngle)
{
	XMMATRIX xmtxWorld = XMLoadFloat4x4(&m_xmf44World);
	XMMATRIX xmtxRotAxis = XMMatrixRotationAxis(XMLoadFloat3(&xmf3RotAxis), (float)(XMConvertToRadians(fAngle)));
	xmtxWorld = xmtxRotAxis * xmtxWorld;
	XMStoreFloat4x4(&m_xmf44World, xmtxWorld);

	m_xv3Right = { m_xmf44World._11, m_xmf44World._12, m_xmf44World._13 };
	m_xv3Up = { m_xmf44World._21, m_xmf44World._22, m_xmf44World._23 };
	m_xv3Look = { m_xmf44World._31, m_xmf44World._32, m_xmf44World._33};
}

void CPlayer::Update(float fTimeElapsed)
{
	/*플레이어의 속도 벡터를 중력 벡터와 더한다. 중력 벡터에 fTimeElapsed를 곱하는 것은 중력을 시간에 비례하도록 적용한다는 의미이다.*/
	m_xv3Velocity.x += m_xv3Gravity.x * fTimeElapsed;
	m_xv3Velocity.y += m_xv3Gravity.y * fTimeElapsed;
	m_xv3Velocity.z += m_xv3Gravity.z * fTimeElapsed;
	/*플레이어의 속도 벡터의 XZ-성분의 크기를 구한다. 이것이 XZ-평면의 최대 속력보다 크면 속도 벡터의 x와 z-방향 성분을 조정한다.*/
	float fLength = sqrtf(m_xv3Velocity.x * m_xv3Velocity.x + m_xv3Velocity.z * m_xv3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ * fTimeElapsed;
	if (fLength > fMaxVelocityXZ)
	{
		m_xv3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xv3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	/*플레이어의 속도 벡터의 Y-성분의 크기를 구한다. 이것이 Y 축 방향의 최대 속력보다 크면 속도 벡터의 y-방향 성분을 조정한다.*/
	fLength = sqrtf(m_xv3Velocity.y * m_xv3Velocity.y);
	float fMaxVelocityY = m_fMaxVelocityY * fTimeElapsed;
	if (fLength > fMaxVelocityY) m_xv3Velocity.y *= (fMaxVelocityY / fLength);

	//플레이어를 속도 벡터 만큼 실제로 이동한다(카메라도 이동될 것이다).
	Move(m_xv3Velocity, false);

	/*플레이어의 위치가 변경될 때 추가로 수행할 작업을 수행한다. 
	예를 들어, 플레이어의 위치가 변경되었지만 플레이어 객체에는 지형(Terrain)의 정보가 없다. 
	플레이어의 새로운 위치가 유효한 위치가 아닐 수도 있고 또는 플레이어의 충돌 검사 등을 수행할 필요가 있다. 
	이러한 상황에서 플레이어의 위치를 유효한 위치로 다시 변경할 수 있다.*/
	if (m_pPlayerUpdatedContext) OnPlayerUpdated(fTimeElapsed);

	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	//플레이어의 위치가 변경되었으므로 카메라의 상태를 갱신한다.
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xv3Position, fTimeElapsed);
	//카메라의 위치가 변경될 때 추가로 수행할 작업을 수행한다.
	if (m_pCameraUpdatedContext) OnCameraUpdated(fTimeElapsed);
	//카메라가 3인칭 카메라이면 카메라가 변경된 플레이어 위치를 바라보도록 한다.
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xv3Position);
	//카메라의 카메라 변환 행렬을 다시 생성한다.
	m_pCamera->RegenerateViewMatrix();

	/*플레이어의 속도 벡터가 마찰력 때문에 감속이 되어야 한다면 감속 벡터를 생성한다. 
	속도 벡터의 반대 방향 벡터를 구하고 단위 벡터로 만든다. 마찰 계수를 시간에 비례하도록 하여 마찰력을 구한다. 
	단위 벡터에 마찰력을 곱하여 감속 벡터를 구한다. 속도 벡터에 감속 벡터를 더하여 속도 벡터를 줄인다. 
	마찰력이 속력보다 크면 속력은 0이 될 것이다.*/

	XMVECTOR xvVelocity = XMLoadFloat3(&m_xv3Velocity);
	XMVECTOR xvDeceleration = -xvVelocity;

	xvDeceleration = XMVector3Normalize(xvDeceleration);
	XMVECTOR xfLength = XMVector3Length(xvVelocity);
	XMStoreFloat(&fLength, xfLength);
	//XMFLOAT3 xv3Deceleration = -m_xv3Velocity;
	//xv3ec3Normalize(&xv3Deceleration, &xv3Deceleration);
	//fLength = xv3ec3Length(&m_xv3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	xvVelocity += xvDeceleration * fDeceleration;

	XMStoreFloat3(&m_xv3Velocity, xvVelocity);
}

CCamera *CPlayer::OnChangeCamera(ID3D11Device *pd3dDevice, DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera *pNewCamera = nullptr;
	//새로운 카메라의 모드에 따라 카메라를 새로 생성한다.
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		pNewCamera = new CFirstPersonCamera(m_pCamera);
		pNewCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		break;
	case THIRD_PERSON_CAMERA:
		pNewCamera = new CThirdPersonCamera(m_pCamera);
		pNewCamera->SetTimeLag(0.25f);
		pNewCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
		break;
	case SPACESHIP_CAMERA:
		pNewCamera = new CSpaceShipCamera(m_pCamera);
		pNewCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 1.0f));
		break;
	}
	/*현재 카메라의 모드가 스페이스-쉽 모드의 카메라이고 새로운 카메라가 1인칭 또는 3인칭 카메라이면 플레이어의 Up 벡터를 월드좌표계의 y-축 방향 벡터(0, 1, 0)이 되도록 한다. 즉, 똑바로 서도록 한다. 그리고 스페이스-쉽 카메라의 경우 플레이어의 이동에는 제약이 없다. 특히, y-축 방향의 움직임이 자유롭다. 그러므로 플레이어의 위치는 공중(위치 벡터의 y-좌표가 0보다 크다)이 될 수 있다. 이때 새로운 카메라가 1인칭 또는 3인칭 카메라이면 플레이어의 위치는 지면이 되어야 한다. 그러므로 플레이어의 Right 벡터와 Look 벡터의 y 값을 0으로 만든다. 이제 플레이어의 Right 벡터와 Look 벡터는 단위벡터가 아니므로 정규화한다.*/
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xv3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_xv3Right.y = 0.0f;
		m_xv3Look.y = 0.0f;
		Chae::XMFloat3Normalize(&m_xv3Right, &m_xv3Right);
		Chae::XMFloat3Normalize(&m_xv3Look, &m_xv3Look);
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		/*Look 벡터와 월드좌표계의 z-축(0, 0, 1)이 이루는 각도(내적=cos)를 계산하여 플레이어의 y-축의 회전 각도 m_fYaw로 설정한다.*/
		m_fYaw = (float)XMConvertToDegrees(acosf(m_xv3Look.z));
		if (m_xv3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		/*새로운 카메라의 모드가 스페이스-쉽 모드의 카메라이고 현재 카메라 모드가 1인칭 또는 3인칭 카메라이면 플레이어의 로컬 축을 현재 카메라의 로컬 축과 같게 만든다.*/
		m_xv3Right = m_pCamera->GetRightVector();
		m_xv3Up    = m_pCamera->GetUpVector();
		m_xv3Look  = m_pCamera->GetLookVector();
	}

	if (pNewCamera)
	{
		//기존 카메라가 없으면 새로운 카메라를 위한 쉐이더 변수를 생성한다.
		if (!m_pCamera) pNewCamera->CreateShaderVariables(pd3dDevice);
		pNewCamera->SetMode(nNewCameraMode);
		//현재 카메라를 사용하는 플레이어 객체를 설정한다.
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera) delete m_pCamera;

	return(pNewCamera);
}

void CPlayer::OnPlayerUpdated(float fTimeElapsed)
{
}

void CPlayer::OnCameraUpdated(float fTimeElapsed)
{
}

void CPlayer::ChangeCamera(ID3D11Device *pd3dDevice, DWORD nNewCameraMode, float fTimeElapsed)
{
}

void CPlayer::OnPrepareRender()
{
	m_xmf44World._11 = m_xv3Right.x;
	m_xmf44World._12 = m_xv3Right.y;
	m_xmf44World._13 = m_xv3Right.z;
	m_xmf44World._21 = m_xv3Up.x;
	m_xmf44World._22 = m_xv3Up.y;
	m_xmf44World._23 = m_xv3Up.z;
	m_xmf44World._31 = m_xv3Look.x;
	m_xmf44World._32 = m_xv3Look.y;
	m_xmf44World._33 = m_xv3Look.z;
	m_xmf44World._41 = m_xv3Position.x;
	m_xmf44World._42 = m_xv3Position.y;
	m_xmf44World._43 = m_xv3Position.z;
}

void CPlayer::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	SetActive(true);
	CAnimatedObject::Render(pd3dDeviceContext, uRenderState, pCamera);
}

void CPlayer::Animate(float fTimeElapsed)
{
	CGameObject::Animate(fTimeElapsed);
	CAnimatedObject::Animate(fTimeElapsed);

	UINT uSize = m_uSize;
	m_uSize = 40.0f;
	vector<CGameObject*> vcArray = QUADMgr.CollisionCheckList(this);
	QUADMgr.ContainedErase();
	
	//QUADMgr.CollisionCheck(this);
	//cout << "에너지 : " << m_nEnergy << endl;
	m_uSize = uSize;
}

///////////////////

CTerrainPlayer::CTerrainPlayer(int nMeshes) : CPlayer(nMeshes)
{
}

CTerrainPlayer::~CTerrainPlayer()
{
}

void CTerrainPlayer::ChangeCamera(ID3D11Device *pd3dDevice, DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return;
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(250.0f);
		//1인칭 카메라일 때 플레이어에 y-축 방향으로 중력이 작용한다.
		SetGravity(XMFLOAT3(0.0f, -300.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(pd3dDevice, FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 1000.0f, ASPECT_RATIO, 60.0f);
		break;
	case SPACESHIP_CAMERA:
		SetFriction(125.0f);
		//스페이스 쉽 카메라일 때 플레이어에 중력이 작용하지 않는다.
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(pd3dDevice, SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 1000.0f, ASPECT_RATIO, 60.0f);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(250.0f);
		//3인칭 카메라일 때 플레이어에 y-축 방향으로 중력이 작용한다.
		SetGravity(XMFLOAT3(0.0f, -300.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(pd3dDevice, THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.55f);
		m_pCamera->SetOffset(XMFLOAT3(0, 20.0f, -50.0)); //XMFLOAT3(0.0f, 30.0f, -30.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 1000.0f, ASPECT_RATIO, 60.0f);
		break;
	default:
		break;
	}
	Update(fTimeElapsed);
}

void CTerrainPlayer::OnPlayerUpdated(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pPlayerUpdatedContext;
	XMFLOAT3 xv3Scale           = pTerrain->GetScale();
	XMFLOAT3 xv3PlayerPosition  = GetPosition();
	int z                       = (int)(xv3PlayerPosition.z / xv3Scale.z);
	bool bReverseQuad = (z % 2);//((z % 2) != 0);
	/*높이 맵에서 플레이어의 현재 위치 (x, z)의 y 값을 구한다. 그리고 플레이어 메쉬의 높이가 12이고 플레이어의 중심이 직육면체의 가운데이므로 y 값에 메쉬의 높이의 절반을 더하면 플레이어의 위치가 된다.*/
	float fHeight = pTerrain->GetHeight(xv3PlayerPosition.x, xv3PlayerPosition.z, bReverseQuad);//+2.0f;
//	cout << "높이는 : " << fHeight << endl;
	/*플레이어의 속도 벡터의 y-값이 음수이면(예를 들어, 중력이 적용되는 경우) 플레이어의 위치 벡터의 y-값이 점점 작아지게 된다.
	이때 플레이어의 현재 위치의 y 값이 지형의 높이(실제로 지형의 높이 + 6)보다 작으면 플레이어가 땅속에 있게 되므로 플레이어의 속도 벡터의 y 값을 0으로 만들고 플레이어의 위치 벡터의 y-값을 지형의 높이로 설정한다. 그러면 플레이어는 지형 위에 있게 된다.*/
	if (xv3PlayerPosition.y < fHeight)
	{
		XMFLOAT3 xv3PlayerVelocity = GetVelocity();
		xv3PlayerVelocity.y = 0.0f;
		SetVelocity(xv3PlayerVelocity);
		xv3PlayerPosition.y = fHeight;
		SetPosition(xv3PlayerPosition);
	}
	//XMFLOAT3 xmf3TerrainNormal = pTerrain->GetNormal(xv3PlayerPosition.x, xv3PlayerPosition.z);
	//XMVECTOR xmvTerrainNormal = XMLoadFloat3(&xmf3TerrainNormal);
	//XMFLOAT3 xmf3RotateAxis;
	//XMStoreFloat3(&xmf3RotateAxis, XMVector3Normalize( XMVector3Cross(xmvTerrainNormal, XMVectorSet(0, 0, 1, 0))));


	//float fAngle = 0.0f;
	//XMStoreFloat(&fAngle, XMVector3Dot(xmvTerrainNormal, XMVectorSet(0, 0, 1, 0)));
	//fAngle = acos(fAngle);

	//CPlayer::Rotate(xmf3RotateAxis, fAngle);
}

void CTerrainPlayer::OnCameraUpdated(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pCameraUpdatedContext;
	XMFLOAT3 xv3Scale           = pTerrain->GetScale();
	CCamera *pCamera            = GetCamera();
	XMFLOAT3 xv3CameraPosition  = pCamera->GetPosition();
	int z                       = (int)(xv3CameraPosition.z / xv3Scale.z);
	bool bReverseQuad = (z % 2);//((z % 2) != 1);
	/*높이 맵에서 카메라의 현재 위치 (x, z)의 높이(y 값)를 구한다. 이 값이 카메라의 위치에 해당하는 지형의 높이 보다 작으면 카메라가 땅속에 있게 된다.
	이렇게 되면 <그림 4>의 왼쪽과 같이 지형이 그려지지 않는 경우가 발생한다(카메라가 지형 안에 있으므로 와인딩 순서가 바뀐다).
	이러한 경우가 발생하지 않도록 카메라의 위치의 최소값은 (지형의 높이 + 5)로 설정한다.
	카메라의 위치의 최소값은 지형의 모든 위치에서 카메라가 지형 아래에 위치하지 않도록 설정한다.*/
	float fHeight = pTerrain->GetHeight(xv3CameraPosition.x, xv3CameraPosition.z, bReverseQuad);// +8.0f;
	if (xv3CameraPosition.y < fHeight)
	{
		xv3CameraPosition.y = fHeight + 2.0f;
		pCamera->SetPosition(xv3CameraPosition);
		//3인칭 카메라의 경우 카메라의 y-위치가 변경되었으므로 카메라가 플레이어를 바라보도록 한다.
		if (pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			CThirdPersonCamera *p3rdPersonCamera = (CThirdPersonCamera *)pCamera;
			XMFLOAT3 look = GetPosition();
			look.y += 0.5f;
			//XMFLOAT3 pos = GetPosition();
			//XMFLOAT3 look = GetLookVector();
			//XMStoreFloat3(&pos, XMLoadFloat3(&pos) + ( XMLoadFloat3(&look) * 60.0f));

			p3rdPersonCamera->SetLookAt(look);
		}
	}
}

CInGamePlayer::CInGamePlayer(int m_nMeshes) : CTerrainPlayer(m_nMeshes)
{
	ZeroMemory(&m_nElemental, sizeof(m_nElemental));

	m_pBuff = nullptr;
	m_pDebuff = nullptr;
}

CInGamePlayer::~CInGamePlayer()
{
	if (m_pBuff) delete m_pBuff;
	if (m_pDebuff) delete m_pDebuff;
}

void CInGamePlayer::BuildObject(CMesh ** ppMeshList, int nMeshes, CTexture * pTexture, CMaterial * pMaterial, CHeightMapTerrain * pTerrain)
{
	if (!m_pBuff) m_pBuff = new CBuff();
	if (!m_pDebuff) m_pDebuff = new CDeBuff();

	for (int i = 0; i < eANI_TOTAL_NUM; ++i)
	{
		CInGamePlayer::SetMesh(ppMeshList[i], i);
	}
	CInGamePlayer::InitializeAnimCycleTime();

	CInGamePlayer::SetMaterial(pMaterial);
	CInGamePlayer::SetTexture(pTexture);

	//플레이어의 위치가 변경될 때 지형의 정보에 따라 플레이어의 위치를 변경할 수 있도록 설정한다.
	CInGamePlayer::SetPlayerUpdatedContext(pTerrain);
	//카메라의 위치가 변경될 때 지형의 정보에 따라 카메라의 위치를 변경할 수 있도록 설정한다.
	CInGamePlayer::SetCameraUpdatedContext(pTerrain);
	/*지형의 xz-평면의 가운데에 플레이어가 위치하도록 한다. 플레이어의 y-좌표가 지형의 높이 보다 크고 중력이 작용하도록 플레이어를 설정하였으므로 플레이어는 점차적으로 하강하게 된다.*/
	CInGamePlayer::InitPosition(XMFLOAT3(pTerrain->GetWidth()*0.5f, pTerrain->GetPeakHeight() + 1000.0f, 300));
}


void CInGamePlayer::GetGameMessage(CGameObject * byObj, eMessage eMSG, void * extra)
{
	static XMFLOAT4 xmfInfo;
	switch (eMSG)
	{
	case eMessage::MSG_GET_SOUL:
		xmfInfo = *(XMFLOAT4*)extra;
		AddEnergy(xmfInfo.w);
		EVENTMgr.InsertDelayMessage(0.0f, MSG_PARTICLE_ON, CGameEventMgr::MSG_TYPE_SCENE, m_pScene, nullptr, extra);
		EVENTMgr.InsertDelayMessage(0.01f, eMessage::MSG_OBJECT_RENEW, CGameEventMgr::MSG_TYPE_OBJECT, byObj);
		//m_pScene->GetGameMessage(nullptr, MSG_PARTICLE_ON, extra);
		return;
	case eMessage::MSG_COLLIDE:
		return;
	case eMessage::MSG_COLLIDED:
		return;
	case eMessage::MSG_NORMAL:
		return;
	case eMessage::MSG_COLLIDE_LOCATION:
		//toObj->GetGameMessage(this, MSG_COLLIDE);
		return;

	case eMessage::MSG_MAGIC_SHOT:
		return;
	}
}

void CInGamePlayer::SendGameMessage(CGameObject * toObj, eMessage eMSG, void * extra)
{
	CGameObject * pObj = nullptr;

	switch (eMSG)
	{
	case eMessage::MSG_NORMAL:
		return;
		// 반대로 메세지 전송하도록 하자
	case eMessage::MSG_COLLIDE:
		//pObj = dynamic_cast<CAbsorbMarble*>(toObj);
		//if (dynamic_cast<CAbsorbMarble*>(toObj))
		//	QUADMgr.DeleteStaticObject(toObj);
		toObj->GetGameMessage(this, MSG_COLLIDED);
		return;
	case eMessage::MSG_COLLIDED:
		toObj->GetGameMessage(this, MSG_COLLIDE);
		return;
	}
}

void CInGamePlayer::InitializeAnimCycleTime()
{
	SetAnimationCycleTime(eANI_IDLE, mfIdleAnim);
	SetAnimationCycleTime(eANI_RUN_FORWARD, mfRunForwardAnim);

	SetAnimationCycleTime(eANI_1H_CAST, mf1HCastAnimTime);
	SetAnimationCycleTime(eANI_1H_MAGIC_ATTACK, mf1HMagicAttackAnimTime);
	SetAnimationCycleTime(eANI_1H_MAGIC_AREA, mf1HMagicAreaAnimTime);
}

void CInGamePlayer::PlayerKeyEventOn(WORD key, void * extra)
{
	//static WORD wdNext[] = { eANI_1H_MAGIC_ATTACK };

	switch (key)
	{
	case 'N':
		ChangeAnimationState(eANI_1H_CAST, true, &mwd1HMagicShot[1], 1);
		EVENTMgr.InsertDelayMessage(mf1HCastAnimTime + 0.6f, eMessage::MSG_MAGIC_SHOT, CGameEventMgr::MSG_TYPE_SCENE, extra);
		return;

	case 'M':
		ChangeAnimationState(eANI_1H_MAGIC_AREA, true, nullptr, 0);
		EVENTMgr.InsertDelayMessage(1.0f, eMessage::MSG_MAGIC_AREA, CGameEventMgr::MSG_TYPE_SCENE, extra);
		return;

	}
}

void CInGamePlayer::AddEnergy(UINT index, UINT num)
{
	++m_nElemental.m_nSum;

	BYTE nNewNum = ++m_nElemental.m_nEnergies[index];
	if (m_nElemental.m_nMinNum < nNewNum)
	{
		m_nElemental.m_nMinNum = nNewNum;
	}

#ifdef _DEBUG
	cout << m_nElemental << endl;
	//cout << "총 에너지 : " << (UINT)m_nElemental.m_nSum << "\t";
	//cout << index << "번 에너지 : " <<  (UINT)m_nElemental.m_nEnergies[index] << endl;
#endif

}

UINT CInGamePlayer::UseEnergy(UINT index, BYTE energyNum, bool bForced)
{
	if (bForced)
	{
		BYTE num = min(m_nElemental.m_nEnergies[index], energyNum);
		m_nElemental.m_nEnergies[index] -= num;
		m_nElemental.m_nSum -= num;
		return num;
	}
	else if (m_nElemental.m_nEnergies[index] >= energyNum)
	{
		m_nElemental.m_nEnergies[index] -= energyNum;
		m_nElemental.m_nSum -= energyNum;
		return energyNum;
	}
	return 0;
}

UINT CInGamePlayer::UseEnergy(UINT energyNum, bool bForced)
{
	if (!bForced || m_nElemental.m_nSum < energyNum)
		return false;
	
	BYTE num = energyNum;

	for (int i = 0; i < ELEMENT_NUM; ++i)
	{
		if (0 == (num -= UseEnergy(i, num, bForced)))
			break;
	}

	cout << m_nElemental << endl;
	return num;
}

UINT CInGamePlayer::UseAllEnergy(UINT energyNum, bool bForced)
{
	if (!bForced || m_nElemental.m_nMinNum < energyNum)
		return false;

	for (int i = 0; i < ELEMENT_NUM; ++i)
		UseEnergy(i, energyNum, bForced);

	return true;
}

PARTILCE_ON_INFO CInGamePlayer::Get1HAnimShotParticleOnInfo()
{
	XMMATRIX mtx = XMLoadFloat4x4(&m_xmf44World);
	mtx = XMMatrixTranslation(0, 9.0f, 10.0f) * mtx;
	XMFLOAT4X4 xmf44Change;
	XMStoreFloat4x4(&xmf44Change, mtx);

	PARTILCE_ON_INFO info;
	info.fColor = 0;
	info.iNum = 4;
	info.m_xmf3Pos = move(XMFLOAT3(xmf44Change._41, xmf44Change._42, xmf44Change._43));
	info.m_xmf3Velocity = move(XMFLOAT3(xmf44Change._31, xmf44Change._32, xmf44Change._33));
	info.m_xmfAccelate = move(XMFLOAT3(-xmf44Change._31, -xmf44Change._32, -xmf44Change._33));

	return info;
}
