#pragma once

#ifndef __SYSTEM_OBJECT
#define __SYSTEM_OBJECT

#include "Object.h"
#include "SystemManager.h"

class CPortalGate : public CGameObject
{
public:
	CPortalGate(int nMeshes);
	virtual ~CPortalGate();

	virtual void GetGameMessage(CEntity * byEntity, eMessage eMSG, void * extra = nullptr);
};

#endif