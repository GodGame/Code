#include "stdafx.h"
#include "Character.h"
#include "AIWarrock.h"

CWarrockIdleState & CWarrockIdleState::GetInstance()
{
	static CWarrockIdleState instance;
	return instance;
}

void CWarrockIdleState::Enter(CWarrock * pWarrock)
{
	pWarrock->ChangeAnimationState(CWarrock::eANI_WARROCK_IDLE, true, nullptr, 0);

	mEvaluator.SetEvaluate(pWarrock->GetTarget(), pWarrock, mfATTACK_RAGNE);
}

void CWarrockIdleState::Execute(CWarrock * pWarrock, float fFrameTime)
{
	//WORD wdAnimState = pWarrock->GetAnimationState();
	mEvaluator.SetEvaluate(pWarrock->GetTarget());

	float fEvaluateValue = mEvaluator.Evaluate();
	if (fEvaluateValue < 1.5f)
	{
		if (fEvaluateValue < 0.3f)
			pWarrock->GetFSM()->ChangeState(&CWarrockPunchState::GetInstance());

		else if (fEvaluateValue < 1.0f)
			pWarrock->GetFSM()->ChangeState(&CWarrockSwipingState::GetInstance());

		else
			pWarrock->GetFSM()->ChangeState(&CWarrockRoarState::GetInstance());
	}
}

void CWarrockIdleState::Exit(CWarrock * pWarrock)
{
}
/////////////////////////////////////////////////////////////////////////////////////
CWarrockPunchState & CWarrockPunchState::GetInstance()
{
	static CWarrockPunchState instance;
	return instance;
}

void CWarrockPunchState::Enter(CWarrock * pWarrock)
{
	pWarrock->ChangeAnimationState(CWarrock::eANI_WARROCK_PUNCH, false, nullptr, 0);

	mEvaluator.SetEvaluate(pWarrock->GetTarget(), pWarrock);
}

void CWarrockPunchState::Execute(CWarrock * pWarrock, float fFrameTime)
{
	if (pWarrock->GetAniMesh()->IsEndAnimation())
	{
		pWarrock->GetFSM()->ChangeState(&CWarrockIdleState::GetInstance());
	}
}

void CWarrockPunchState::Exit(CWarrock * pWarrock)
{
}
////////////////////////////////////////////////////////////////////////////////////
CWarrockSwipingState & CWarrockSwipingState::GetInstance()
{
	static CWarrockSwipingState instance;
	return instance;
}

void CWarrockSwipingState::Enter(CWarrock * pWarrock)
{
	pWarrock->ChangeAnimationState(CWarrock::eANI_WARROCK_SWIPING, false, nullptr, 0);

	mEvaluator.SetEvaluate(pWarrock->GetTarget(), pWarrock);
}

void CWarrockSwipingState::Execute(CWarrock * pWarrock, float fFrameTime)
{
	if (pWarrock->GetAniMesh()->IsEndAnimation())
	{
		pWarrock->GetFSM()->ChangeState(&CWarrockIdleState::GetInstance());
	}
}

void CWarrockSwipingState::Exit(CWarrock * pWarrock)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
CWarrockRoarState & CWarrockRoarState::GetInstance()
{
	static CWarrockRoarState instance;
	return instance;
}

void CWarrockRoarState::Enter(CWarrock * pWarrock)
{
	pWarrock->ChangeAnimationState(CWarrock::eANI_WARROCK_ROAR, false, nullptr, 0);
}

void CWarrockRoarState::Execute(CWarrock * pWarrock, float fFrameTime)
{
	if (pWarrock->GetAniMesh()->IsEndAnimation())
	{
		pWarrock->GetFSM()->ChangeState(&CWarrockChaseTargetState::GetInstance());
	}
}

void CWarrockRoarState::Exit(CWarrock * pWarrock)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
CWarrockChaseTargetState & CWarrockChaseTargetState::GetInstance()
{
	static CWarrockChaseTargetState instance;
	return instance;
}

void CWarrockChaseTargetState::Enter(CWarrock * pWarrock)
{
	pWarrock->ChangeAnimationState(CWarrock::eANI_WARROCK_RUN, false, nullptr, 0);

	mEvaluator.SetEvaluate(pWarrock->GetTarget(), pWarrock, mfCHASE_RAGNE);
}

void CWarrockChaseTargetState::Execute(CWarrock * pWarrock, float fFrameTime)
{
	CGameObject * pTarget = pWarrock->GetTarget();
	mEvaluator.SetEvaluate(pTarget);

	float fEvaluateValue = mEvaluator.Evaluate();

	if (fEvaluateValue < 0.05f)
	{
		pWarrock->GetFSM()->ChangeState(&CWarrockSwipingState::GetInstance());
	}
	else if (fEvaluateValue < 1.0f)
	{
		XMVECTOR ToTarget = XMLoadFloat3(&pTarget->GetPosition()) - XMLoadFloat3(&pWarrock->GetPosition());
		ToTarget = XMVector3Normalize(ToTarget) * 10.0f;// *fFrameTime;
		
		XMFLOAT3 velocity; 
		XMStoreFloat3(&velocity, ToTarget);
		pWarrock->Move(velocity, true);
		pWarrock->LookToTarget(pTarget);
	}
	else
	{
		pWarrock->GetFSM()->ChangeState(&CWarrockIdleState::GetInstance());
	}
}

void CWarrockChaseTargetState::Exit(CWarrock * pWarrock)
{
	pWarrock->SetVelocity(XMFLOAT3(0, 0, 0));
}
