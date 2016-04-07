#include "stdafx.h"
#include "MyInline.h"
#include "Character.h"
#include "AIWarrock.h"


void CCharacter::SetExternalPower(XMFLOAT3 & xmf3Power)
{
	m_xv3ExternalPower.x += xmf3Power.x;
	m_xv3ExternalPower.y += xmf3Power.y;
	m_xv3ExternalPower.z += xmf3Power.z;
}

void CCharacter::CalculateFriction(float fTimeElapsed)
{

	/*�÷��̾��� �ӵ� ���Ͱ� ������ ������ ������ �Ǿ�� �Ѵٸ� ���� ���͸� �����Ѵ�.
	�ӵ� ������ �ݴ� ���� ���͸� ���ϰ� ���� ���ͷ� �����. ���� ����� �ð��� ����ϵ��� �Ͽ� �������� ���Ѵ�.
	���� ���Ϳ� �������� ���Ͽ� ���� ���͸� ���Ѵ�. �ӵ� ���Ϳ� ���� ���͸� ���Ͽ� �ӵ� ���͸� ���δ�.
	�������� �ӷº��� ũ�� �ӷ��� 0�� �� ���̴�.*/
	float fDeceleration = (m_fFriction * fTimeElapsed);
	float fLength = 0.0f;
	float fy = m_xv3Velocity.y;
	m_xv3Velocity.y = 0.f;
	XMVECTOR xvVelocity = XMLoadFloat3(&m_xv3Velocity);
	XMVECTOR xvDeceleration = XMVector3Normalize(-xvVelocity);
	XMStoreFloat(&fLength, XMVector3Length(xvVelocity));

	if (fDeceleration > fLength) fDeceleration = fLength;
	xvVelocity += xvDeceleration * fDeceleration;

	XMStoreFloat3(&m_xv3Velocity, xvVelocity);
	m_xv3Velocity.y = fy;

	// �ܺ��� �� ������ ���
	fDeceleration = (m_fFriction * fTimeElapsed);
	xvVelocity = XMLoadFloat3(&m_xv3ExternalPower);
	XMStoreFloat(&fLength, XMVector3LengthSq(xvVelocity));
	if (fLength > 0.0f)
	{
		xvDeceleration = XMVector3Normalize(-xvVelocity);
		XMStoreFloat(&fLength, XMVector3Length(xvVelocity));

		if (fDeceleration > fLength) fDeceleration = fLength;
		xvVelocity += xvDeceleration * fDeceleration;
		XMStoreFloat3(&m_xv3ExternalPower, xvVelocity);
	}
}

CCharacter::CCharacter(int nMeshes) : CAnimatedObject(nMeshes)
{
	m_xv3Position     = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_xv3Right        = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xv3Up           = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xv3Look         = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xv3ExternalPower = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fGravity        = 0.0f;
	m_fMaxVelocityXZ  = 0.0f;
	m_fMaxVelocityY   = 0.0f;
	m_fFriction       = 0.0f;

	m_pUpdatedContext = nullptr;
}

CCharacter::~CCharacter()
{
}

void CCharacter::OnPrepareRender()
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

void CCharacter::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	OnPrepareRender();
	CAnimatedObject::Render(pd3dDeviceContext, uRenderState, pCamera);
}

void CCharacter::Animate(float fTimeElapsed)
{
	CAnimatedObject::Animate(fTimeElapsed);
}

void CCharacter::SetPosition(float fx, float fy, float fz)
{
	XMFLOAT3 pos(fx, fy, fz);
	CCharacter::SetPosition(pos);
}

void CCharacter::SetPosition(XMFLOAT3& xv3Position)
{
	XMFLOAT3 xv3Result;
	Chae::XMFloat3Sub(&xv3Result, &xv3Position, &m_xv3Position);
	Move(xv3Result, false);
}

void CCharacter::Move(ULONG nDirection, float fDistance, bool bVelocity)
{

}

void CCharacter::Move(XMFLOAT3 & xv3Shift, bool bUpdateVelocity)
{
	//bUpdateVelocity�� ���̸� �̵����� �ʰ� �ӵ� ���͸� �����Ѵ�.
	if (bUpdateVelocity)
		Chae::XMFloat3Add(&m_xv3Velocity, &m_xv3Velocity, &xv3Shift);

	else
		Chae::XMFloat3Add(&m_xv3Position, &m_xv3Position, &xv3Shift);
}

void CCharacter::Move(float fxOffset, float fyOffset, float fzOffset)
{
}

void CCharacter::Rotate(float x, float y, float z)
{
	XMVECTOR xmvRight = XMLoadFloat3(&m_xv3Right);
	XMVECTOR xmvUp = XMLoadFloat3(&m_xv3Up);
	XMVECTOR xmvLook = XMLoadFloat3(&m_xv3Look);
	XMMATRIX mtxRotate;

	if (x != 0.0f)
	{
		mtxRotate = XMMatrixRotationAxis(xmvRight, (float)XMConvertToRadians(x));
		xmvLook   = XMVector3TransformNormal(xmvLook, mtxRotate);
		xmvUp     = XMVector3TransformNormal(xmvUp, mtxRotate);
	}
	if (y != 0.0f)
	{
		mtxRotate = XMMatrixRotationAxis(xmvUp, (float)XMConvertToRadians(y));
		xmvLook   = XMVector3TransformNormal(xmvLook, mtxRotate);
		xmvRight  = XMVector3TransformNormal(xmvRight, mtxRotate);
	}
	if (z != 0.0f)
	{
		mtxRotate = XMMatrixRotationAxis(xmvLook, (float)XMConvertToRadians(z));
		xmvUp     = XMVector3TransformNormal(xmvUp, mtxRotate);
		xmvRight  = XMVector3TransformNormal(xmvRight, mtxRotate);
	}

	xmvLook  = XMVector3Normalize(xmvLook);
	xmvRight = XMVector3Cross(xmvUp, xmvLook);
	xmvRight = XMVector3Normalize(xmvRight);
	xmvUp    = XMVector3Cross(xmvLook, xmvRight);
	xmvUp    = XMVector3Normalize(xmvUp);

	XMStoreFloat3(&m_xv3Right, xmvRight);
	XMStoreFloat3(&m_xv3Up, xmvUp);
	XMStoreFloat3(&m_xv3Look, xmvLook);
}

void CCharacter::Rotate(XMFLOAT3 & xmf3RotAxis, float fAngle)
{
	XMMATRIX xmtxWorld = XMLoadFloat4x4(&m_xmf44World);
	XMMATRIX xmtxRotAxis = XMMatrixRotationAxis(XMLoadFloat3(&xmf3RotAxis), (float)(XMConvertToRadians(fAngle)));
	xmtxWorld = xmtxRotAxis * xmtxWorld;
	XMStoreFloat4x4(&m_xmf44World, xmtxWorld);

	m_xv3Right = { m_xmf44World._11, m_xmf44World._12, m_xmf44World._13 };
	m_xv3Up    = { m_xmf44World._21, m_xmf44World._22, m_xmf44World._23 };
	m_xv3Look  = { m_xmf44World._31, m_xmf44World._32, m_xmf44World._33 };
}

void CCharacter::LookToTarget(CGameObject * pTarget)
{
	float fLookY = m_xv3Look.y;

	Chae::XMFloat3TargetToNormal(&m_xv3Look, &pTarget->GetPosition(), &GetPosition());

	m_xv3Look.y = fLookY;

	Chae::XMFloat3Cross(&m_xv3Right, &m_xv3Up, &m_xv3Look);
}

void CCharacter::Update(float fTimeElapsed)
{
	/*�÷��̾��� �ӵ� ���͸� �߷� ���Ϳ� ���Ѵ�. �߷� ���Ϳ� fTimeElapsed�� ���ϴ� ���� �߷��� �ð��� ����ϵ��� �����Ѵٴ� �ǹ��̴�.*/
	m_xv3Velocity.y += m_fGravity * fTimeElapsed;
	/*�÷��̾��� �ӵ� ������ XZ-������ ũ�⸦ ���Ѵ�. �̰��� XZ-����� �ִ� �ӷº��� ũ�� �ӵ� ������ x�� z-���� ������ �����Ѵ�.*/
	float fLength = 0.0f; // = sqrtf(m_xv3Velocity.x * m_xv3Velocity.x + m_xv3Velocity.z * m_xv3Velocity.z);
	XMStoreFloat(&fLength, XMVector2Length(XMLoadFloat3(&m_xv3Velocity)));
	//XMStoreFloat3(&m_xv3Velocity, velocity);

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

	XMFLOAT3 xmfVelocity = m_xv3Velocity;
	XMStoreFloat3(&m_xv3Velocity, XMLoadFloat3(&m_xv3Velocity) + XMLoadFloat3(&m_xv3ExternalPower) * fTimeElapsed);

	Move(m_xv3Velocity, false);
	m_xv3Velocity = xmfVelocity;

	if (m_pUpdatedContext) OnContextUpdated(fTimeElapsed);

	CCharacter::CalculateFriction(fTimeElapsed);
}

void CCharacter::OnContextUpdated(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pUpdatedContext;
	XMFLOAT3 xv3Scale = pTerrain->GetScale();
	XMFLOAT3 xv3Position = GetPosition();
	int z = (int)(xv3Position.z / xv3Scale.z);
	bool bReverseQuad = (z % 2);//((z % 2) != 0);
								
	float fHeight = pTerrain->GetHeight(xv3Position.x, xv3Position.z, bReverseQuad);

	if (xv3Position.y < fHeight)
	{
		XMFLOAT3 xv3PlayerVelocity = GetVelocity();
		xv3PlayerVelocity.y = 0.0f;
		SetVelocity(xv3PlayerVelocity);
		xv3Position.y = fHeight;
		SetPosition(xv3Position);
	}
}

void CCharacter::Attack(CCharacter * pToChar, short stDamage)
{
}

void CCharacter::AttackSuccess(CCharacter * pToChar, short stDamage)
{
	pToChar->Damaged(pToChar, stDamage);
}

void CCharacter::Damaged(CCharacter * pByChar, short stDamage)
{
	m_Status.Damaged(stDamage);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMonster::CMonster(int nMeshes) : CCharacter(nMeshes)
{
	SetGravity(-50);
	SetMaxVelocityY(50.0f);
	m_pTarget = nullptr;
}

CMonster::~CMonster()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSkeleton::CSkeleton(int nMeshes) : CMonster(nMeshes)
{
}

CSkeleton::~CSkeleton()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CWarrock::CWarrock(int nMeshes) : CMonster(nMeshes)
{
}

CWarrock::~CWarrock()
{
}

void CWarrock::BuildObject(CCharacter * pTarget)
{
	m_Status.ResetStatus();
	m_Status.SetHP(mfMAX_HEALTH);

	SetMaxVelocityXZ(30.0f);
	SetFriction(20.0f);

	m_pTarget = pTarget;

	mDotEvaluator.SetEvaluate(pTarget, this);

	m_pStateMachine = new CStateMachine<CWarrock>(this);
	m_pStateMachine->SetCurrentState(&CWarrockIdleState::GetInstance());
}

void CWarrock::InitializeAnimCycleTime()
{
	SetAnimationCycleTime(CWarrock::eANI_WARROCK_IDLE,    mfIDLE_ANIM);
	SetAnimationCycleTime(CWarrock::eANI_WARROCK_RUN,     mfRUN_ANIM);
	SetAnimationCycleTime(CWarrock::eANI_WARROCK_ROAR,    mfROAR_ANIM);
	SetAnimationCycleTime(CWarrock::eANI_WARROCK_PUNCH,   mfPUNCH_ANIM);
	SetAnimationCycleTime(CWarrock::eANI_WARROCK_SWIPING, mfSWIP_ANIM);
	SetAnimationCycleTime(CWarrock::eANI_WARROCK_DEATH,   mfDEATH_ANIM);
}

void CWarrock::Animate(float fTimeElapsed)
{
	CAnimatedObject::Animate(fTimeElapsed);
	m_pStateMachine->Update(fTimeElapsed);
	CCharacter::Update(fTimeElapsed);
}

void CWarrock::GetGameMessage(CEntity * byObj, eMessage eMSG, void * extra)
{
	eWarrockAnim eAnim = eWarrockAnim::eANI_WARROCK_IDLE;

	switch (eMSG)
	{
#if 0
	case eMessage::MSG_OBJECT_ANIM_CHANGE:
		eAnim = eWarrockAnim(*static_cast<int*>(extra));

		switch (*static_cast<int*>(extra))
		{
		case eANI_WARROCK_PUNCH:
		case eANI_WARROCK_SWIPING:
		case eANI_WARROCK_ROAR:
			ChangeAnimationState(eAnim, true, nullptr, 0);
			break;
		}
		return;
#endif
	case eMessage::MSG_CULL_IN:
		m_bActive = true;
		return;

	case eMessage::MSG_OBJECT_STATE_CHANGE:
		m_pStateMachine->ChangeState(static_cast<CAIState<CWarrock>*>(extra));
		return;
	}
}

void CWarrock::Attack(CCharacter * pToChar, short stDamage)
{
}

void CWarrock::AttackSuccess(CCharacter * pToChar, short stDamage)
{
	pToChar->Damaged(pToChar, stDamage);
}

void CWarrock::Damaged(CCharacter * pByChar, short stDamage)
{
	m_Status.Damaged(stDamage);
}
