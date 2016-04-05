#include "stdafx.h"
#include "StatePlayer.h"
#include "Player.h"

CPlayerIdleState & CPlayerIdleState::GetInstance()
{
	static CPlayerIdleState instance;
	return instance;
}

void CPlayerIdleState::Enter(CInGamePlayer * pPlayer)
{
	pPlayer->ChangeAnimationState(eANI_IDLE, false, nullptr, 0);
}

void CPlayerIdleState::Execute(CInGamePlayer * pPlayer, float fFrameTime)
{
}

void CPlayerIdleState::Exit(CInGamePlayer * pPlayer)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////
CPlayerCastState & CPlayerCastState::GetInstance()
{
	static CPlayerCastState instance;
	return instance;
}

void CPlayerCastState::Enter(CInGamePlayer * pPlayer)
{
	pPlayer->ChangeAnimationState(eANI_1H_CAST, false, nullptr, 0);
}

void CPlayerCastState::Execute(CInGamePlayer * pPlayer, float fFrameTime)
{
}

void CPlayerCastState::Exit(CInGamePlayer * pPlayer)
{
}
//////////////////////////////////////////////////////////////////////////////////////////////
CPlayerUseMagicState & CPlayerUseMagicState::GetInstance()
{
	static CPlayerUseMagicState instance;
	return instance;
}

void CPlayerUseMagicState::Enter(CInGamePlayer * pPlayer)
{
}

void CPlayerUseMagicState::Execute(CInGamePlayer * pPlayer, float fFrameTime)
{
}

void CPlayerUseMagicState::Exit(CInGamePlayer * pPlayer)
{
}
///////////////////////////////////////////////////////////////////////////////////////////
CPlayerDamagedState & CPlayerDamagedState::GetInstance()
{
	static CPlayerDamagedState instance;
	return instance;
}

void CPlayerDamagedState::Enter(CInGamePlayer * pPlayer)
{
	pPlayer->ChangeAnimationState(eANI_DAMAGED_FRONT_01, false, nullptr, 0);
}

void CPlayerDamagedState::Execute(CInGamePlayer * pPlayer, float fFrameTime)
{
	if (pPlayer->GetAniMesh()->IsEndAnimation())
	{
		pPlayer->GetFSM()->ChangeState(&CPlayerIdleState::GetInstance());
	}
}

void CPlayerDamagedState::Exit(CInGamePlayer * pPlayer)
{
	pPlayer->GetStatus().SetCanMove(true);
	pPlayer->GetStatus().SetUnbeatable(false);
}
///////////////////////////////////////////////////////////////////////////////////
CPlayerKnockbackState & CPlayerKnockbackState::GetInstance()
{
	static CPlayerKnockbackState instance;
	return instance;
}

void CPlayerKnockbackState::Enter(CInGamePlayer * pPlayer)
{
	XMFLOAT3 xmf3LookInverse = pPlayer->GetLookVectorInverse();
	xmf3LookInverse.y = 0.0f;

	XMVECTOR xmvLookInverse =  XMLoadFloat3(&xmf3LookInverse);
	xmvLookInverse = XMVector3Normalize(xmvLookInverse) * mfKnockBackLength;
	XMStoreFloat3(&xmf3LookInverse, xmvLookInverse);
	pPlayer->SetExternalPower(xmf3LookInverse);

	pPlayer->ChangeAnimationState(eANI_DAMAGED_FRONT_02, false, nullptr, 0);
	cout << "HP : " << pPlayer->GetStatus().GetHP() << endl;
}

void CPlayerKnockbackState::Execute(CInGamePlayer * pPlayer, float fFrameTime)
{
	if (pPlayer->GetAniMesh()->IsEndAnimation())
	{
		pPlayer->GetFSM()->ChangeState(&CPlayerIdleState::GetInstance());
	}
}

void CPlayerKnockbackState::Exit(CInGamePlayer * pPlayer)
{
	pPlayer->GetStatus().SetCanMove(true);
	pPlayer->GetStatus().SetUnbeatable(false);
}
//////////////////////////////////////////////////////////////////////////////
CPlayerDeathState & CPlayerDeathState::GetInstance()
{
	static CPlayerDeathState instance;
	return instance;
}

void CPlayerDeathState::Enter(CInGamePlayer * pPlayer)
{
	pPlayer->GetStatus().SetCanMove(false);
	pPlayer->GetStatus().SetAilive(false);
	pPlayer->ChangeAnimationState(eANI_DEATH_FRONT, false, nullptr, 0);
}

void CPlayerDeathState::Execute(CInGamePlayer * pPlayer, float fFrameTime)
{
	if (pPlayer->GetAniMesh()->IsEndAnimation())
	{
		pPlayer->GetAniMesh()->Stop();
	}
}

void CPlayerDeathState::Exit(CInGamePlayer * pPlayer)
{
	pPlayer->GetStatus().SetCanMove(true);
	pPlayer->GetStatus().SetAilive(true);
}
