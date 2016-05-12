#include "stdafx.h"
#include "StatePlayer.h"
#include "Player.h"
//#include "SystemManager.h"

CPlayerIdleState & CPlayerIdleState::GetInstance()
{
	static CPlayerIdleState instance;
	return instance;
}

void CPlayerIdleState::Enter(CInGamePlayer * pPlayer)
{
	pPlayer->ChangeAnimationState(eANI_IDLE, false, nullptr, 0);
	pPlayer->SetCancled(false);
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
	const CEntity * pEntity = pPlayer->GetDamagedEntity();
	XMVECTOR xmvFromVector = XMLoadFloat3(&pPlayer->GetPosition()) - XMLoadFloat3(&pEntity->GetPosition());
	xmvFromVector *= XMVectorSet(1, 0, 1, 1);
	xmvFromVector = XMVector3Normalize(xmvFromVector);
	xmvFromVector *= mfKnockBackLength;

	XMFLOAT3 xmf3Power;
	XMStoreFloat3(&xmf3Power, xmvFromVector);
	pPlayer->SetExternalPower(xmf3Power);

	pPlayer->ChangeAnimationState(eANI_DAMAGED_FRONT_02, false, nullptr, 0);
	pPlayer->LookToTarget(pEntity);
	//cout << "HP : " << pPlayer->GetStatus().GetHP() << endl;

	if (SYSTEMMgr.GetPlayerNum() == pPlayer->GetPlayerNum())
	{
		EVENTMgr.InsertDelayMessage(0.0f, eMessage::MSG_EFFECT_RADIAL_ON, CGameEventMgr::MSG_TYPE_SCENE, pPlayer->GetScene());
		//GetGameMessage(nullptr, MSG_EFFECT_VIBRATE_ON, nullptr);
		EVENTMgr.InsertDelayMessage(0.3f, eMessage::MSG_EFFECT_RADIAL_OFF, CGameEventMgr::MSG_TYPE_SCENE, pPlayer->GetScene());
		EVENTMgr.InsertDelayMessage(0.1f, eMessage::MSG_EFFECT_VIBRATE_ON, CGameEventMgr::MSG_TYPE_ENTITY, pPlayer);
		EVENTMgr.InsertDelayMessage(0.4f, eMessage::MSG_EFFECT_VIBRATE_OFF, CGameEventMgr::MSG_TYPE_ENTITY, pPlayer);
	}
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
///////////////////////////////////////////////////////////////////////////////
CPlayerDominateState & CPlayerDominateState::GetInstance()
{
	static CPlayerDominateState instance;
	return instance;
}

void CPlayerDominateState::Enter(CInGamePlayer * pPlayer)
{
	WORD aniList[] = {eANI_BLOCK_IDLE};
	pPlayer->ChangeAnimationState(eANI_BLOCK_START, false, aniList, 1);

	EVENTMgr.InsertDelayMessage(SYSTEMMgr.GetDominateSpendTime(), 
		eMessage::MSG_PLAYER_DOMIATE_END, CGameEventMgr::MSG_TYPE_ENTITY, pPlayer);
}

void CPlayerDominateState::Execute(CInGamePlayer * pPlayer, float fFrameTime)
{
	if (pPlayer->GetAnimationState() == eANI_BLOCK_END)
	{
		if (pPlayer->GetAniMesh()->IsEndAnimation())
		{
			pPlayer->GetFSM()->ChangeState(&CPlayerIdleState::GetInstance());
		}
	}
}

void CPlayerDominateState::Exit(CInGamePlayer * pPlayer)
{
	//pPlayer->StopDominate();
}

//////////////////////////////////////////////////////////////////////////////
CPlayerDeathState & CPlayerDeathState::GetInstance()
{
	static CPlayerDeathState instance;
	return instance;
}

void CPlayerDeathState::Enter(CInGamePlayer * pPlayer)
{
	m_bCanChangeState = false;
	pPlayer->GetStatus().SetHP(0);
	pPlayer->GetStatus().SetCanMove(false);
	pPlayer->GetStatus().SetAlive(false);
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
	m_bCanChangeState = true;
	pPlayer->GetStatus().SetCanMove(true);
	pPlayer->GetStatus().SetAlive(true);
}
