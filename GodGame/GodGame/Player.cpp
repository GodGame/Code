#include "stdafx.h"
#include "MyInline.h"
#include "Player.h"
#include "Scene.h"

CPlayer::CPlayer(int nMeshes) : CDynamicObject(nMeshes)
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

	m_nEnergy = 0;

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
	//�÷��̾��� ���� ī�޶��� UpdateShaderVariables() ��� �Լ��� ȣ���Ѵ�.
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
		XMVECTOR xv3Shift = XMVectorSet(0, 0, 0, 0);
		//ȭ��ǥ Ű ���衯�� ������ ���� z-�� �������� �̵�(����)�Ѵ�. ���顯�� ������ �ݴ� �������� �̵��Ѵ�.
		if (dwDirection & DIR_FORWARD) xv3Shift += XMLoadFloat3(&m_xv3Look) * fDistance;
		if (dwDirection & DIR_BACKWARD) xv3Shift -= XMLoadFloat3(&m_xv3Look) * fDistance;
		//ȭ��ǥ Ű ���桯�� ������ ���� x-�� �������� �̵��Ѵ�. ���硯�� ������ �ݴ� �������� �̵��Ѵ�.
		if (dwDirection & DIR_RIGHT) xv3Shift += XMLoadFloat3(&m_xv3Right) * fDistance;
		if (dwDirection & DIR_LEFT) xv3Shift -= XMLoadFloat3(&m_xv3Right) * fDistance;
		//��Page Up���� ������ ���� y-�� �������� �̵��Ѵ�. ��Page Down���� ������ �ݴ� �������� �̵��Ѵ�.
		if (dwDirection & DIR_UP) xv3Shift += XMLoadFloat3(&m_xv3Up) * fDistance;
		if (dwDirection & DIR_DOWN) xv3Shift -= XMLoadFloat3(&m_xv3Up) * fDistance;

		XMFLOAT3 xmf3Shift;
		XMStoreFloat3(&xmf3Shift, xv3Shift);
		//�÷��̾ ���� ��ġ ���Ϳ��� xv3Shift ���� ��ŭ �̵��Ѵ�.
		Move(xmf3Shift, bUpdateVelocity);
	}
}
void CPlayer::Move(XMFLOAT3& xv3Shift, bool bUpdateVelocity)
{
	//bUpdateVelocity�� ���̸� �÷��̾ �̵����� �ʰ� �ӵ� ���͸� �����Ѵ�.
	if (bUpdateVelocity)
	{
		m_xv3Velocity.x += xv3Shift.x;
		m_xv3Velocity.y += xv3Shift.y;
		m_xv3Velocity.z += xv3Shift.z;
	}
	else
	{
		//�÷��̾ ���� ��ġ ���Ϳ��� xv3Shift ���� ��ŭ �̵��Ѵ�.
		XMFLOAT3 xv3Position;
		xv3Position.x = m_xv3Position.x + xv3Shift.x;
		xv3Position.y = m_xv3Position.y + xv3Shift.y;
		xv3Position.z = m_xv3Position.z + xv3Shift.z;

		m_xv3Position = xv3Position;
		//�÷��̾��� ��ġ�� ����Ǿ����Ƿ� ī�޶��� ��ġ�� xv3Shift ���� ��ŭ �̵��Ѵ�.
		m_pCamera->Move(xv3Shift);
	}
}

//�÷��̾ ���� x-��, y-��, z-���� �߽����� ȸ���Ѵ�.
void CPlayer::Rotate(float x, float y, float z)
{
	XMVECTOR xmvRight = XMLoadFloat3(&m_xv3Right);
	XMVECTOR xmvUp = XMLoadFloat3(&m_xv3Up);
	XMVECTOR xmvLook = XMLoadFloat3(&m_xv3Look);

	XMMATRIX mtxRotate;
	DWORD nCurrentCameraMode = m_pCamera->GetMode();

	//1��Ī ī�޶� �Ǵ� 3��Ī ī�޶��� ��� �÷��̾��� ȸ���� �ణ�� ������ ������.
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA))
	{
		/*���� x-���� �߽����� ȸ���ϴ� ���� ���� �յڷ� ���̴� ���ۿ� �ش��Ѵ�. �׷��Ƿ� x-���� �߽����� ȸ���ϴ� ������ -89.0~+89.0�� ���̷� �����Ѵ�. x�� ������ m_fPitch���� ���� ȸ���ϴ� �����̹Ƿ� x��ŭ ȸ���� ���� Pitch�� +89�� ���� ũ�ų� -89�� ���� ������ m_fPitch�� +89�� �Ǵ� -89���� �ǵ��� ȸ������(x)�� �����Ѵ�.*/
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		//���� y-���� �߽����� ȸ���ϴ� ���� ������ ������ ���̹Ƿ� ȸ�� ������ ������ ����.
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		/*���� z-���� �߽����� ȸ���ϴ� ���� ������ �¿�� ����̴� ���̹Ƿ� ȸ�� ������ -20.0~+20.0�� ���̷� ���ѵȴ�. z�� ������ m_fRoll���� ���� ȸ���ϴ� �����̹Ƿ� z��ŭ ȸ���� ���� m_fRoll�� +20�� ���� ũ�ų� -20������ ������ m_fRoll�� +20�� �Ǵ� -20���� �ǵ��� ȸ������(z)�� �����Ѵ�.*/
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		//ī�޶� x, y, z ��ŭ ȸ���Ѵ�. �÷��̾ ȸ���ϸ� ī�޶� ȸ���ϰ� �ȴ�.
		m_pCamera->Rotate(x, y, z);

		/*�÷��̾ ȸ���Ѵ�. 1��Ī ī�޶� �Ǵ� 3��Ī ī�޶󿡼� �÷��̾��� ȸ���� ���� y-�࿡���� �Ͼ��. �÷��̾��� ���� y-��(Up ����)�� �������� ���� z-��(Look ����)�� ���� x-��(Right ����)�� ȸ����Ų��. �⺻������ Up ���͸� �������� ȸ���ϴ� ���� �÷��̾ �ȹٷ� ���ִ� ���� �����Ѵٴ� �ǹ��̴�.*/
		if (y != 0.0f)
		{
			mtxRotate = XMMatrixRotationAxis(xmvUp, (float)XMConvertToRadians(y));
			xmvLook = XMVector3TransformNormal(xmvLook, mtxRotate);
			xmvRight = XMVector3TransformNormal(xmvRight, mtxRotate);
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		/*�����̽�-�� ī�޶󿡼� �÷��̾��� ȸ���� ȸ�� ������ ������ ����. �׸��� ��� ���� �߽����� ȸ���� �� �� �ִ�.*/
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

	/*ȸ������ ���� �÷��̾��� ���� x-��, y-��, z-���� ���� �������� ���� �� �����Ƿ� z-��(LookAt ����)�� �������� �Ͽ� ���� �����ϰ� �������Ͱ� �ǵ��� �Ѵ�.*/
	xmvLook  = XMVector3Normalize(xmvLook);
	xmvRight = XMVector3Cross(xmvUp, xmvLook);
	xmvRight = XMVector3Normalize(xmvRight);
	xmvUp    = XMVector3Cross(xmvLook, xmvRight);
	xmvUp    = XMVector3Normalize(xmvUp);

	XMStoreFloat3(&m_xv3Right, xmvRight);
	XMStoreFloat3(&m_xv3Up, xmvUp);
	XMStoreFloat3(&m_xv3Look, xmvLook);
}

void CPlayer::Update(float fTimeElapsed)
{
	/*�÷��̾��� �ӵ� ���͸� �߷� ���Ϳ� ���Ѵ�. �߷� ���Ϳ� fTimeElapsed�� ���ϴ� ���� �߷��� �ð��� ����ϵ��� �����Ѵٴ� �ǹ��̴�.*/
	m_xv3Velocity.x += m_xv3Gravity.x * fTimeElapsed;
	m_xv3Velocity.y += m_xv3Gravity.y * fTimeElapsed;
	m_xv3Velocity.z += m_xv3Gravity.z * fTimeElapsed;
	/*�÷��̾��� �ӵ� ������ XZ-������ ũ�⸦ ���Ѵ�. �̰��� XZ-����� �ִ� �ӷº��� ũ�� �ӵ� ������ x�� z-���� ������ �����Ѵ�.*/
	float fLength = sqrtf(m_xv3Velocity.x * m_xv3Velocity.x + m_xv3Velocity.z * m_xv3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ * fTimeElapsed;
	if (fLength > fMaxVelocityXZ)
	{
		m_xv3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xv3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	/*�÷��̾��� �ӵ� ������ Y-������ ũ�⸦ ���Ѵ�. �̰��� Y �� ������ �ִ� �ӷº��� ũ�� �ӵ� ������ y-���� ������ �����Ѵ�.*/
	fLength = sqrtf(m_xv3Velocity.y * m_xv3Velocity.y);
	float fMaxVelocityY = m_fMaxVelocityY * fTimeElapsed;
	if (fLength > fMaxVelocityY) m_xv3Velocity.y *= (fMaxVelocityY / fLength);

	//�÷��̾ �ӵ� ���� ��ŭ ������ �̵��Ѵ�(ī�޶� �̵��� ���̴�).
	Move(m_xv3Velocity, false);

	/*�÷��̾��� ��ġ�� ����� �� �߰��� ������ �۾��� �����Ѵ�. 
	���� ���, �÷��̾��� ��ġ�� ����Ǿ����� �÷��̾� ��ü���� ����(Terrain)�� ������ ����. 
	�÷��̾��� ���ο� ��ġ�� ��ȿ�� ��ġ�� �ƴ� ���� �ְ� �Ǵ� �÷��̾��� �浹 �˻� ���� ������ �ʿ䰡 �ִ�. 
	�̷��� ��Ȳ���� �÷��̾��� ��ġ�� ��ȿ�� ��ġ�� �ٽ� ������ �� �ִ�.*/
	if (m_pPlayerUpdatedContext) OnPlayerUpdated(fTimeElapsed);

	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	//�÷��̾��� ��ġ�� ����Ǿ����Ƿ� ī�޶��� ���¸� �����Ѵ�.
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xv3Position, fTimeElapsed);
	//ī�޶��� ��ġ�� ����� �� �߰��� ������ �۾��� �����Ѵ�.
	if (m_pCameraUpdatedContext) OnCameraUpdated(fTimeElapsed);
	//ī�޶� 3��Ī ī�޶��̸� ī�޶� ����� �÷��̾� ��ġ�� �ٶ󺸵��� �Ѵ�.
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xv3Position);
	//ī�޶��� ī�޶� ��ȯ ����� �ٽ� �����Ѵ�.
	m_pCamera->RegenerateViewMatrix();

	/*�÷��̾��� �ӵ� ���Ͱ� ������ ������ ������ �Ǿ�� �Ѵٸ� ���� ���͸� �����Ѵ�. 
	�ӵ� ������ �ݴ� ���� ���͸� ���ϰ� ���� ���ͷ� �����. ���� ����� �ð��� ����ϵ��� �Ͽ� �������� ���Ѵ�. 
	���� ���Ϳ� �������� ���Ͽ� ���� ���͸� ���Ѵ�. �ӵ� ���Ϳ� ���� ���͸� ���Ͽ� �ӵ� ���͸� ���δ�. 
	�������� �ӷº��� ũ�� �ӷ��� 0�� �� ���̴�.*/

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
	//���ο� ī�޶��� ��忡 ���� ī�޶� ���� �����Ѵ�.
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
	/*���� ī�޶��� ��尡 �����̽�-�� ����� ī�޶��̰� ���ο� ī�޶� 1��Ī �Ǵ� 3��Ī ī�޶��̸� �÷��̾��� Up ���͸� ������ǥ���� y-�� ���� ����(0, 1, 0)�� �ǵ��� �Ѵ�. ��, �ȹٷ� ������ �Ѵ�. �׸��� �����̽�-�� ī�޶��� ��� �÷��̾��� �̵����� ������ ����. Ư��, y-�� ������ �������� �����Ӵ�. �׷��Ƿ� �÷��̾��� ��ġ�� ����(��ġ ������ y-��ǥ�� 0���� ũ��)�� �� �� �ִ�. �̶� ���ο� ī�޶� 1��Ī �Ǵ� 3��Ī ī�޶��̸� �÷��̾��� ��ġ�� ������ �Ǿ�� �Ѵ�. �׷��Ƿ� �÷��̾��� Right ���Ϳ� Look ������ y ���� 0���� �����. ���� �÷��̾��� Right ���Ϳ� Look ���ʹ� �������Ͱ� �ƴϹǷ� ����ȭ�Ѵ�.*/
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xv3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_xv3Right.y = 0.0f;
		m_xv3Look.y = 0.0f;
		Chae::XMFloat3Normalize(&m_xv3Right, &m_xv3Right);
		Chae::XMFloat3Normalize(&m_xv3Look, &m_xv3Look);
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		/*Look ���Ϳ� ������ǥ���� z-��(0, 0, 1)�� �̷�� ����(����=cos)�� ����Ͽ� �÷��̾��� y-���� ȸ�� ���� m_fYaw�� �����Ѵ�.*/
		m_fYaw = (float)XMConvertToDegrees(acosf(m_xv3Look.z));
		if (m_xv3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		/*���ο� ī�޶��� ��尡 �����̽�-�� ����� ī�޶��̰� ���� ī�޶� ��尡 1��Ī �Ǵ� 3��Ī ī�޶��̸� �÷��̾��� ���� ���� ���� ī�޶��� ���� ��� ���� �����.*/
		m_xv3Right = m_pCamera->GetRightVector();
		m_xv3Up    = m_pCamera->GetUpVector();
		m_xv3Look  = m_pCamera->GetLookVector();
	}

	if (pNewCamera)
	{
		//���� ī�޶� ������ ���ο� ī�޶� ���� ���̴� ������ �����Ѵ�.
		if (!m_pCamera) pNewCamera->CreateShaderVariables(pd3dDevice);
		pNewCamera->SetMode(nNewCameraMode);
		//���� ī�޶� ����ϴ� �÷��̾� ��ü�� �����Ѵ�.
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
	CGameObject::Render(pd3dDeviceContext, uRenderState, pCamera);
}

void CPlayer::Animate(float fTimeElapsed)
{
	UINT uSize = m_uSize;
	m_uSize = 40.0f;
	//vector<CGameObject*> vcArray = QUADMgr.CollisionCheckList(this);
	//QUADMgr.ContainedErase();
	QUADMgr.CollisionCheck(this);
	cout << "������ : " << m_nEnergy << endl;
	m_uSize = uSize;
}

void CPlayer::GetGameMessage(CGameObject * byObj, eMessage eMSG, void * extra)
{
	switch (eMSG)
	{
	case eMessage::MSG_GET_SOUL:
		++m_nEnergy;
		m_pScene->GetGameMessage(nullptr, MSG_PARTICLE_ON, extra);
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
	}
}

void CPlayer::SendGameMessage(CGameObject * toObj, eMessage eMSG, void * extra)
{
	CGameObject * pObj = nullptr;

	switch (eMSG)
	{
	case eMessage::MSG_NORMAL:
		return;
		// �ݴ�� �޼��� �����ϵ��� ����
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
		//1��Ī ī�޶��� �� �÷��̾ y-�� �������� �߷��� �ۿ��Ѵ�.
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
		//�����̽� �� ī�޶��� �� �÷��̾ �߷��� �ۿ����� �ʴ´�.
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
		//3��Ī ī�޶��� �� �÷��̾ y-�� �������� �߷��� �ۿ��Ѵ�.
		SetGravity(XMFLOAT3(0.0f, -300.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(pd3dDevice, THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.35f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
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
	bool bReverseQuad = !(z % 2);//((z % 2) != 0);
	/*���� �ʿ��� �÷��̾��� ���� ��ġ (x, z)�� y ���� ���Ѵ�. �׸��� �÷��̾� �޽��� ���̰� 12�̰� �÷��̾��� �߽��� ������ü�� ����̹Ƿ� y ���� �޽��� ������ ������ ���ϸ� �÷��̾��� ��ġ�� �ȴ�.*/
	float fHeight = pTerrain->GetHeight(xv3PlayerPosition.x, xv3PlayerPosition.z, bReverseQuad) + 6.0f;
//	cout << "���̴� : " << fHeight << endl;
	/*�÷��̾��� �ӵ� ������ y-���� �����̸�(���� ���, �߷��� ����Ǵ� ���) �÷��̾��� ��ġ ������ y-���� ���� �۾����� �ȴ�.
	�̶� �÷��̾��� ���� ��ġ�� y ���� ������ ����(������ ������ ���� + 6)���� ������ �÷��̾ ���ӿ� �ְ� �ǹǷ� �÷��̾��� �ӵ� ������ y ���� 0���� ����� �÷��̾��� ��ġ ������ y-���� ������ ���̷� �����Ѵ�. �׷��� �÷��̾�� ���� ���� �ְ� �ȴ�.*/
	if (xv3PlayerPosition.y < fHeight)
	{
		XMFLOAT3 xv3PlayerVelocity = GetVelocity();
		xv3PlayerVelocity.y = 0.0f;
		SetVelocity(xv3PlayerVelocity);
		xv3PlayerPosition.y = fHeight;
		SetPosition(xv3PlayerPosition);
	}
}

void CTerrainPlayer::OnCameraUpdated(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pCameraUpdatedContext;
	XMFLOAT3 xv3Scale           = pTerrain->GetScale();
	CCamera *pCamera            = GetCamera();
	XMFLOAT3 xv3CameraPosition  = pCamera->GetPosition();
	int z                       = (int)(xv3CameraPosition.z / xv3Scale.z);
	bool bReverseQuad = !(z % 2);//((z % 2) != 1);
	/*���� �ʿ��� ī�޶��� ���� ��ġ (x, z)�� ����(y ��)�� ���Ѵ�. �� ���� ī�޶��� ��ġ�� �ش��ϴ� ������ ���� ���� ������ ī�޶� ���ӿ� �ְ� �ȴ�.
	�̷��� �Ǹ� <�׸� 4>�� ���ʰ� ���� ������ �׷����� �ʴ� ��찡 �߻��Ѵ�(ī�޶� ���� �ȿ� �����Ƿ� ���ε� ������ �ٲ��).
	�̷��� ��찡 �߻����� �ʵ��� ī�޶��� ��ġ�� �ּҰ��� (������ ���� + 5)�� �����Ѵ�.
	ī�޶��� ��ġ�� �ּҰ��� ������ ��� ��ġ���� ī�޶� ���� �Ʒ��� ��ġ���� �ʵ��� �����Ѵ�.*/
	float fHeight = pTerrain->GetHeight(xv3CameraPosition.x, xv3CameraPosition.z, bReverseQuad) + 8.0f;
	if (xv3CameraPosition.y < fHeight)
	{
		xv3CameraPosition.y = fHeight;
		pCamera->SetPosition(xv3CameraPosition);
		//3��Ī ī�޶��� ��� ī�޶��� y-��ġ�� ����Ǿ����Ƿ� ī�޶� �÷��̾ �ٶ󺸵��� �Ѵ�.
		if (pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			CThirdPersonCamera *p3rdPersonCamera = (CThirdPersonCamera *)pCamera;
			p3rdPersonCamera->SetLookAt(GetPosition());
		}
	}
}

CInGamePlayer::CInGamePlayer(int m_nMeshes)
{
	ZeroMemory(&m_nEnergies, sizeof(m_nEnergies));

	m_pBuff = nullptr;
	m_pDebuff = nullptr;
}

CInGamePlayer::~CInGamePlayer()
{
	if (m_pBuff) delete m_pBuff;
	if (m_pDebuff) delete m_pDebuff;
}

void CInGamePlayer::BuildObject()
{
	if (!m_pBuff) m_pBuff = new CBuff();
	if (!m_pDebuff) m_pDebuff = new CDeBuff();
}
