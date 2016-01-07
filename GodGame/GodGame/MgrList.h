#pragma once

#ifndef __MGR_LIST
#define __MGR_LIST

#include "TextureMgr.h"
#include "StatesMgr.h"

class CManagers
{
public:
	static void BuildManagers(ID3D11Device * pd3dDevice)
	{
		TXMgr.BuildResources(pd3dDevice);
		MaterialMgr.BuildResources(pd3dDevice);
	}
};

#endif