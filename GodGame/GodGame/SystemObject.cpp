#include "stdafx.h"
#include "SystemObject.h"
#include "Player.h"

CPortalGate::CPortalGate(int nMeshes) : CGameObject(nMeshes)
{
}

CPortalGate::~CPortalGate()
{
}

void CPortalGate::GetGameMessage(CEntity * byEntity, eMessage eMSG, void * extra)
{
	CPlayer * pPlayer = dynamic_cast<CPlayer*>(byEntity);
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
		if (pPlayer) cout << pPlayer->GetPlayerNum() << "번 플레이어 입니다." << endl;
		return;
	}
}
