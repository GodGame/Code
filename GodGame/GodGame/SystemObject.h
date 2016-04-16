#pragma once

#ifndef __SYSTEM_OBJECT
#define __SYSTEM_OBJECT

//#define PLAYER_01_COLOR "WhiteLight"

#include "Object.h"
#include "SystemManager.h"

class CPortalGate : public CGameObject
{
	const XMFLOAT3 m_xmf3OffsetZonePos{ -0.5f, 37.f, 0.f };

public:
	CPortalGate(int nMeshes);
	virtual ~CPortalGate();

	virtual void GetGameMessage(CEntity * byEntity, eMessage eMSG, void * extra = nullptr);
	XMFLOAT3 GetZonePosition() { XMFLOAT3 pos = GetPosition(); return XMFLOAT3(pos.x + m_xmf3OffsetZonePos.x, pos.y + m_xmf3OffsetZonePos.y, pos.z + m_xmf3OffsetZonePos.z); }
};

#endif