#include "stdafx.h"
#include "Character.h"
#include "AIWarrock.h"
#include "Player.h"
#include "StatePlayer.h"

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
	CCharacter * pChar = pWarrock->GetTarget();
	if (false == pChar->GetStatus().IsCanDamaged()) return;

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
	pWarrock->LookToTarget(pWarrock->GetTarget());
	mEvaluator.SetEvaluate(pWarrock->GetTarget(), pWarrock, mfMAX_RAGNE);
}

void CWarrockPunchState::Execute(CWarrock * pWarrock, float fFrameTime)
{
	CAnimatedMesh * pMesh = pWarrock->GetAniMesh();
	CInGamePlayer * pTarget = static_cast<CInGamePlayer*>(pWarrock->GetTarget());

	if (pMesh->IsEndAnimation())
	{
		pWarrock->GetFSM()->ChangeState(&CWarrockDelayState::GetInstance());
	}
	else if (pTarget->GetStatus().IsCanDamaged())
	{
		float fIndexPercent = (static_cast<float>(pMesh->GetAnimIndex()) / static_cast<float>(pMesh->GetAnimationAllIndex()));
		if (fIndexPercent < 0.3f || fIndexPercent > 0.8f) return;

		mEvaluator.SetEvaluate(pWarrock->GetTarget());
		float fResult = mEvaluator.Evaluate();
		// -70~70도 사이
		if (fResult > COS_70)
		{
			pWarrock->AttackSuccess(pTarget, pWarrock->GetPunchDamage());
		}
	}
}

void CWarrockPunchState::Exit(CWarrock * pWarrock)
{
	EVENTMgr.InsertDelayMessage(m_fDelay, eMessage::MSG_OBJECT_STATE_CHANGE,
		CGameEventMgr::MSG_TYPE_ENTITY, pWarrock, nullptr, &CWarrockIdleState::GetInstance());
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
	CAnimatedMesh * pMesh = pWarrock->GetAniMesh();
	CInGamePlayer * pTarget = static_cast<CInGamePlayer*>(pWarrock->GetTarget());

	if (pMesh->IsEndAnimation())
	{
		pWarrock->GetFSM()->ChangeState(&CWarrockDelayState::GetInstance());
	}
	else if (pTarget->GetStatus().IsCanDamaged())
	{
		float fIndexPercent = (static_cast<float>(pMesh->GetAnimIndex()) / static_cast<float>(pMesh->GetAnimationAllIndex()));
		if (fIndexPercent < 0.5f || fIndexPercent > 0.8f) return;

		mEvaluator.SetEvaluate(pWarrock->GetTarget());
		float fResult = mEvaluator.Evaluate();
		// -70~70도 사이
		if (fResult > COS_70)
		{
			pWarrock->AttackSuccess(pTarget, pWarrock->GetSwipingDamage());
		}
	}
}

void CWarrockSwipingState::Exit(CWarrock * pWarrock)
{
	EVENTMgr.InsertDelayMessage(m_fDelay, eMessage::MSG_OBJECT_STATE_CHANGE,
		CGameEventMgr::MSG_TYPE_ENTITY, pWarrock, nullptr, &CWarrockIdleState::GetInstance());
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

CWarrockDelayState & CWarrockDelayState::GetInstance()
{
	static CWarrockDelayState instance;
	return instance;
}

void CWarrockDelayState::Enter(CWarrock * pWarrock)
{
	pWarrock->ChangeAnimationState(CWarrock::eANI_WARROCK_IDLE, false, nullptr, 0);
}

void CWarrockDelayState::Execute(CWarrock * pWarrock, float fFrameTime)
{
}

void CWarrockDelayState::Exit(CWarrock * pWarrock)
{
}
