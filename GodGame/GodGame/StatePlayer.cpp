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
	pPlayer->GetStatus().Damaged(10.0f);
	cout << "HP : " << pPlayer->GetStatus().GetHP() << endl;
	pPlayer->ChangeAnimationState(eANI_DAMAGED_FRONT_02, false, nullptr, 0);
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
}

CPlayerDeathState & CPlayerDeathState::GetInstance()
{
	static CPlayerDeathState instance;
	return instance;
}

void CPlayerDeathState::Enter(CInGamePlayer * pPlayer)
{
	pPlayer->ChangeAnimationState(eANI_DEATH_FRONT, false, nullptr, 0);
}

void CPlayerDeathState::Execute(CInGamePlayer * pPlayer, float fFrameTime)
{
}

void CPlayerDeathState::Exit(CInGamePlayer * pPlayer)
{
}
